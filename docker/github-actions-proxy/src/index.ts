import { Command } from 'commander';

import { cloneRepository, checkoutRepository } from './git-utils';
import { parseGitHubActionFile, parseGitHubActionInputs } from './github-actions-parser';
import { runActions } from './github-actions-runner';
import { createDirectory, directoryExists } from './io-utils';
import { parseActionSource, parseActionInput, ParsedActionInputResult } from './options-parser';
import { isNullOrEmpty } from './string-utils';

const program = new Command('github actions proxy');

program
    .version('1.0.0')
    .requiredOption('-a, --action <action>', 'GitHub Action to use.')
    .option('-i, --input <input...>', 'GitHub Action input variable KEY=VALUE.')
    .option('-d, --action-download-dir <dir>', 'Directory to store GitHub Action source code.', 'actions');

program.parse(process.argv);

parseAction(program.opts()).catch((reason: any) => {
    console.error(reason.message);
});

interface ParsedProgramResult {
    action: string;
    input?: string[];
    actionDownloadDir: string;
}

async function parseAction(options: ParsedProgramResult) {
    if (isNullOrEmpty(process.env['ACTIONS_CACHE_URL']) || isNullOrEmpty(process.env['ACTIONS_RUNTIME_TOKEN'])) {
        throw new Error('This program requires ACTIONS_CACHE_URL and ACTIONS_RUNTIME_TOKEN environment variable to be available.');
    }

    console.log('Executing GitHub Actions\n');

    // Parse Action Source
    const actionSource = parseActionSource(options.action);

    if (actionSource === null) {
        throw new Error('Invalid Action Source');
    }

    console.log(`uses: ${options.action}`);

    // Parse Action Inputs
    const actionInputs: ParsedActionInputResult[] = [];

    if (options.input != null) {
        console.log('with:');

        for (let index = 0; index < options.input.length; index++) {
            const actionInput = await parseActionInput(options.input[index]);
            
            if (actionInput != null) {
                actionInputs.push(actionInput);
                console.info(`    ${actionInput.key}: ${actionInput.value}`);
            }
        }
    }

    console.log();

    // First make a directory where we can clone the GitHub Actions.
    if (!await directoryExists(options.actionDownloadDir)) {
        await createDirectory(options.actionDownloadDir);
    }

    const repoDir = `${options.actionDownloadDir}/${actionSource.repo}`;

    if (!await directoryExists(repoDir)) {
        await cloneRepository(`https://github.com/${actionSource?.owner}/${actionSource?.repo}`, repoDir);
    }

    await checkoutRepository(actionSource.ref, repoDir);

    console.log();

    // Parse action.yml in the target directory.
    let targetDir = repoDir;
    let actionFile = repoDir;

    if (!isNullOrEmpty(actionSource.filename)) {
        targetDir += `/${actionSource.filename}`;
        actionFile += `/${actionSource.filename}`;
    }

    actionFile += '/action.yml';

    const actionMetadata = parseGitHubActionFile(actionFile);

    if (actionMetadata == null) {
        throw new Error('Invalid Action Source. Reason: action.yml not found.');
    }

    const requiredInputs: string[] = [];
    
    if (actionMetadata.inputs != null) {
        Object.entries(actionMetadata.inputs).forEach(([id, input]) => {
            if (input.required) {
                requiredInputs.push(id);
            }
        });
    }
    
    const missingInputs: string[] = [];

    requiredInputs.forEach(input => {
        if (!actionInputs.find(element => element.key === input)) {
            missingInputs.push(input);
        }
    });

    if (missingInputs.length > 0) {
        throw new Error(`Missing inputs: ${missingInputs.join(', ')}`);
    }

    const environmentVariable = parseGitHubActionInputs(actionInputs, actionMetadata);

    if (actionMetadata.runs.using !== 'node20') {
        throw new Error(`Selected action is using an unsupported running mode.`);
    }
    
    const targetScript = `${targetDir}/${actionMetadata.runs.main}`;

    await runActions(targetScript, environmentVariable);
}