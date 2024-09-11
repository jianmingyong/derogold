"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const os_1 = require("os");
const commander_1 = require("commander");
const child_process_1 = require("child_process");
// Function to run a shell command and stream output line by line
function runShellCommand(command, args) {
    const child = (0, child_process_1.spawn)(command, args);
    // Stream the stdout data (output) line by line
    child.stdout.on('data', (data) => {
        process.stdout.write(data.toString()); // Write each chunk of data to stdout
    });
    // Handle any error output
    child.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
    });
    // Handle the exit event
    child.on('close', (code) => {
        console.log(`Child process exited with code ${code}`);
    });
}
const program = new commander_1.Command('github actions proxy');
program
    .version('1.0.0')
    .requiredOption('-a, --action <action>', 'GitHub action source')
    .option('-i, --input <input...>', 'input variable KEY=VALUE')
    .option('-t, --test', 'testing mode');
program.parse(process.argv);
const opts = program.opts();
if (opts.test && process.env['ACTIONS_CACHE_URL'] === '' || process.env['ACTIONS_RUNTIME_TOKEN'] === '') {
    console.error('This program requires ACTIONS_CACHE_URL and ACTIONS_RUNTIME_TOKEN environment variable to be available.');
    process.exit(1);
}
console.info((0, os_1.tmpdir)());
