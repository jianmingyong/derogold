import { tmpdir } from 'os';
import { Command } from 'commander';
import { spawn } from 'child_process';

// Function to run a shell command and stream output line by line
function runShellCommand(command: string, args: string[]) {
    const child = spawn(command, args);

    // Stream the stdout data (output) line by line
    child.stdout.on('data', (data: Buffer) => {
        process.stdout.write(data.toString()); // Write each chunk of data to stdout
    });

    // Handle any error output
    child.stderr.on('data', (data: Buffer) => {
        console.error(`stderr: ${data}`);
    });

    // Handle the exit event
    child.on('close', (code: number) => {
        console.log(`Child process exited with code ${code}`);
    });
}

const program = new Command('github actions proxy');

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

console.info(tmpdir());