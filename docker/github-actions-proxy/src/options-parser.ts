import * as crypto from 'crypto';
import * as fs from 'fs';
import * as glob from 'glob';

export interface ParsedActionSourceResult {
    owner: string;
    repo: string;
    filename?: string;
    ref: string;
}

export function parseActionSource(input: string): ParsedActionSourceResult | null {
    const regex = /^([a-zA-Z0-9][a-zA-Z0-9.-]{0,38})\/([a-zA-Z0-9._-]+)(\/([^@]+))?@([a-fA-F0-9]{40}|[a-zA-Z0-9._/-]+)$/;
    const match = input.match(regex);

    if (match) {
        const [, owner, repo, , filename, ref] = match;
        return {
            owner,
            repo,
            filename: filename || undefined,
            ref,
        };
    } else {
        return null;
    }
}

export interface ParsedActionInputResult {
    key: string;
    value: string;
}

/**
 * Parses a string of the form `inputId=inputValue` where inputValue can contain GitHub Actions expressions.
 * @param input - The input string to parse.
 * @returns An object with inputId and inputValue with GitHub Actions expressions replaced.
 */
export async function parseActionInput(input: string): Promise<ParsedActionInputResult | null> {
    const regex = /^([a-zA-Z_][a-zA-Z0-9_-]*)=([^]*)$/;

    // Execute the regex to extract inputId and inputValue
    const match = input.match(regex);

    if (match) {
        const key = match[1];
        let value = match[2].trim();

        const expressionRegex = /\$\{\{\s*(.*?)\s*\}\}/g;
        const expressionMatches = value.matchAll(expressionRegex);
        const replaceResult: string[] = [];

        for (const match of expressionMatches) {
            const hashFilesGlob = parseHashFilesCall(match[1]);

            if (hashFilesGlob.length > 0) {
                replaceResult.push(await hashFiles(hashFilesGlob));
            } else {
                replaceResult.push('');
            }
        }

        // Replace GitHub Actions expressions with a placeholder or dynamic value
        value = replaceExpressions(value, _expression => {
            for (let index = 0; index < replaceResult.length; index++) {
                return replaceResult[index];
            }
            return '';
        });

        return { key, value };
    }

    // Return null if the input does not match the expected format
    return null;
}

/**
 * Replaces GitHub Actions expressions of the form `${{ <expression> }}` in the input string.
 * @param input - The input string containing expressions.
 * @param replaceFn - A function that takes an expression and returns the replacement text.
 * @returns The string with expressions replaced.
 */
function replaceExpressions(input: string, replaceFn: (expression: string) => string): string {
    // Regular expression to match `${{ <expression> }}`
    const regex = /\$\{\{\s*(.*?)\s*\}\}/g;

    // Replace each expression with the result from replaceFn
    return input.replace(regex, (_, expression) => replaceFn(expression));
}

/**
 * Parses a string to extract file patterns from a `hashFiles` function call.
 * @param input - The input string containing `hashFiles(path[, path2, ...])`.
 * @returns An array of extracted file patterns, or an empty array if parsing fails.
 */
function parseHashFilesCall(input: string): string[] {
    // Regular expression to match `hashFiles(path1[, path2, ...])`
    const regex = /^hashFiles\(([^)]+)\)$/;

    // Execute the regex to match and extract the paths
    const match = input.match(regex);

    if (match) {
        // Extract the paths string
        const pathsString = match[1].trim();
        // Split the paths string by commas, removing surrounding quotes and extra spaces
        return pathsString
            .split(',')
            .map(path => path.trim().replace(/^["']|["']$/g, ''))
            .filter(path => path.length > 0);
    }

    // Return an empty array if the input does not match the expected format
    return [];
}

/**
 * Calculates a SHA-256 hash for the set of files that match the given path pattern(s).
 * @param patterns - A single path pattern or multiple patterns separated by commas.
 * @returns A SHA-256 hash for the set of files, or an empty string if no files are matched.
 */
async function hashFiles(patterns: string[]): Promise<string> {
    const files = await glob.glob(patterns, { nodir: true });

    if (files.length === 0) {
        return '';
    }

    // Helper function to calculate SHA-256 hash of a file
    async function hashFile(filePath: string): Promise<string> {
        return new Promise((resolve, reject) => {
            const hash = crypto.createHash('sha256');
            const stream = fs.createReadStream(filePath);

            stream.on('data', (data) => hash.update(data));
            stream.on('end', () => resolve(hash.digest('hex')));
            stream.on('error', (err) => reject(err));
        });
    }

    // Calculate hashes for each file
    const fileHashes = await Promise.all(files.map(file => hashFile(file)));

    // Combine all file hashes and calculate a final hash
    const combinedHash = crypto.createHash('sha256');
    fileHashes.forEach(hash => combinedHash.update(hash));

    return combinedHash.digest('hex');
}