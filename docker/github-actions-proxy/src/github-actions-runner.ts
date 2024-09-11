import { spawn } from 'child_process';
import process from 'process';

interface ParsedCommand {
    command: string;
    parameters?: Record<string, string>;
    value: string;
}

function parseGitHubCommand(commandString: string): ParsedCommand | null {
    // Regular expression to match the command format
    const regex = /^::(\w+)(?:\s+([^:]*))?::(.*)$/;

    // Execute the regex
    const match = commandString.match(regex);

    if (match) {
        const [_, command, parametersString, value] = match;

        // Parse parameters
        const parameters: Record<string, string> = {};
        if (parametersString) {
            parametersString.split(',').forEach(param => {
                const [key, val] = param.split('=').map(s => s.trim());
                if (key && val) {
                    parameters[key] = val;
                }
            });
        }

        return {
            command,
            parameters,
            value
        };
    }

    // Return null if the command does not match the format
    return null;
}

/**
 * Masks the value by replacing each word with asterisks of the same length.
 * @param value - The string to mask.
 * @returns The masked string with each word replaced by asterisks.
 */
function maskValue(value: string): string {
    // Split the string into words by whitespace
    const words = value.split(/\s+/);

    // Replace each word with asterisks of the same length
    const maskedWords = words.map(_word => '*');

    // Join the masked words back into a string with the original whitespace
    return maskedWords.join(' ');
}

export function runActions(path: string, env: Record<string, string>): Promise<void> {
    const outputBuffer = function (data: Buffer) {
        const result = data.toString();
        const command = parseGitHubCommand(result);

        if (command) {
            if (command.command === 'debug') {
                console.info(`[Debug] ${command.value}`);
            } else if (command.command === 'add-mask') {
                console.info(maskValue(command.value))
            } else {
                console.info(result);
            }
        } else {
            console.info(result);
        }
    };

    return new Promise((resolve, reject) => {
        const nodeProcess = spawn('node', [path], {
            env: {
                ...process.env,
                ...env
            },
            stdio: ['ignore', 'pipe', 'pipe'],
        });

        nodeProcess.stdout.on('data', outputBuffer);
        nodeProcess.stderr.on('data', outputBuffer);

        nodeProcess.on('error', (error) => {
            reject(error);
        });

        nodeProcess.on('exit', (code) => {
            if (code === 0) {
                resolve();
            } else {
                reject(new Error(`Action failed with exit code ${code}`));
            }
        });
    });
}