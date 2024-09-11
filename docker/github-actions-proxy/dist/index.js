"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
Object.defineProperty(exports, "__esModule", { value: true });
const commander_1 = require("commander");
const git_utils_1 = require("./git-utils");
const github_actions_parser_1 = require("./github-actions-parser");
const github_actions_runner_1 = require("./github-actions-runner");
const io_utils_1 = require("./io-utils");
const options_parser_1 = require("./options-parser");
const string_utils_1 = require("./string-utils");
const program = new commander_1.Command('github actions proxy');
program
    .version('1.0.0')
    .requiredOption('-a, --action <action>', 'GitHub action source')
    .option('-i, --input <input...>', 'input variable KEY=VALUE')
    .option('-d, --action-download-dir <dir>', 'Directory to store GitHub action source', 'actions');
program.parse(process.argv);
parseAction(program.opts()).catch((reason) => {
    console.error(reason.message);
});
function parseAction(options) {
    return __awaiter(this, void 0, void 0, function* () {
        var _a, _b;
        if ((0, string_utils_1.isNullOrEmpty)(process.env['ACTIONS_CACHE_URL']) || (0, string_utils_1.isNullOrEmpty)(process.env['ACTIONS_RUNTIME_TOKEN'])) {
            throw new Error('This program requires ACTIONS_CACHE_URL and ACTIONS_RUNTIME_TOKEN environment variable to be available.');
        }
        console.info('Executing GitHub Actions\n');
        // Parse Action Source
        const actionSource = (0, options_parser_1.parseActionSource)(options.action);
        if (actionSource == null) {
            throw new Error('Invalid Action Source');
        }
        console.info(`uses: ${options.action}`);
        // Parse Action Inputs
        const actionInputs = [];
        if (options.input != null) {
            console.info('with:');
            for (let index = 0; index < options.input.length; index++) {
                const actionInput = yield (0, options_parser_1.parseActionInput)(options.input[index]);
                if (actionInput != null) {
                    actionInputs.push(actionInput);
                    console.info(`    ${actionInput.key}: ${actionInput.value}`);
                }
            }
        }
        console.info();
        // First make a directory where we can clone the GitHub Actions.
        if (!(yield (0, io_utils_1.directoryExists)(options.actionDownloadDir))) {
            yield (0, io_utils_1.createDirectory)(options.actionDownloadDir);
        }
        const repoDir = `${options.actionDownloadDir}/${actionSource.repo}`;
        if (!(yield (0, io_utils_1.directoryExists)(repoDir))) {
            yield (0, git_utils_1.cloneRepository)(`https://github.com/${actionSource === null || actionSource === void 0 ? void 0 : actionSource.owner}/${actionSource === null || actionSource === void 0 ? void 0 : actionSource.repo}`, repoDir);
        }
        yield (0, git_utils_1.checkoutRepository)(actionSource.ref, repoDir);
        console.info();
        // Parse action.yml in the target directory.
        let targetDir = repoDir;
        let actionFile = repoDir;
        if (!(0, string_utils_1.isNullOrEmpty)(actionSource.filename)) {
            targetDir += `/${actionSource.filename}`;
            actionFile += `/${actionSource.filename}`;
        }
        actionFile += '/action.yml';
        const actionMetadata = (0, github_actions_parser_1.parseGitHubActionFile)(actionFile);
        if (actionMetadata == null) {
            throw new Error('Invalid Action Source. Reason: action.yml not found.');
        }
        const requiredInputs = [];
        if (actionMetadata.inputs != null) {
            Object.entries(actionMetadata.inputs).forEach(([id, input]) => {
                if (input.required) {
                    requiredInputs.push(id);
                }
            });
        }
        const missingInputs = [];
        requiredInputs.forEach(input => {
            if (!actionInputs.find(element => element.key === input)) {
                missingInputs.push(input);
            }
        });
        if (missingInputs.length > 0) {
            throw new Error(`Missing inputs: ${missingInputs.join(', ')}`);
        }
        const environmentVariable = (0, github_actions_parser_1.parseGitHubActionInputs)(actionInputs, actionMetadata);
        environmentVariable['ACTIONS_CACHE_URL'] = (_a = process.env['ACTIONS_CACHE_URL']) !== null && _a !== void 0 ? _a : '';
        environmentVariable['ACTIONS_RUNTIME_TOKEN'] = (_b = process.env['ACTIONS_RUNTIME_TOKEN']) !== null && _b !== void 0 ? _b : '';
        if (actionMetadata.runs.using !== 'node20') {
            throw new Error(`Selected action is using an unsupported running mode.`);
        }
        const targetScript = `${targetDir}/${actionMetadata.runs.main}`;
        yield (0, github_actions_runner_1.runActions)(targetScript, environmentVariable);
    });
}
//# sourceMappingURL=index.js.map