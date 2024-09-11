import fs from 'fs';
import yaml from 'js-yaml';

import { ParsedActionInputResult } from './options-parser';

export interface ActionMetadata {
    name: string;
    author?: string;
    description: string;
    inputs?: {
        [id: string]: {
            description: string;
            required?: boolean;
            default?: string;
            deprecationMessage?: string;
        };
    };
    outputs?: {
        [id: string]: {
            description: string;
            value: string;
        };
    };
    runs: {
        using: string;
        main: string;
        pre?: string;
        'pre-if'?: string;
        post?: string;
        'post-if'?: string;
    };
    branding?: {
        color: 'white' | 'black' | 'yellow' | 'blue' | 'green' | 'orange' | 'red' | 'purple' | 'gray-dark';
        icon: string;
    }
}

export function parseGitHubActionFile(file: string): ActionMetadata | null {
    try {
        // Read the file synchronously
        const fileContent = fs.readFileSync(file, 'utf8');

        // Parse the YAML content into a JavaScript object
        return yaml.load(fileContent) as ActionMetadata;
    } catch (e) {
        return null;
    }
}

export function parseGitHubActionInputs(inputs: ParsedActionInputResult[], actionMetadata: ActionMetadata): Record<string, string> {
    if (actionMetadata.inputs == null) {
        return {};
    }

    const result: Record<string, string> = {};

    Object.entries(actionMetadata.inputs).forEach(([id, input]) => {
        const actionInput = inputs.find(element => element.key === id);

        if (actionInput == null) {
            if (input.default != null) {
                result[`INPUT_${id.replace(' ', '_').toUpperCase()}`] = input.default;
            }
        } else {
            result[`INPUT_${id.replace(' ', '_').toUpperCase()}`] = actionInput.value;
        }
    });

    return result;
}