import { access, constants, mkdir, stat } from 'fs';
import { promisify } from 'util';

export async function directoryExists(path: string): Promise<boolean> {
    try {
        // Check if the path is a directory
        await promisify(access)(path, constants.F_OK); // F_OK checks for the existence
        const stats = await promisify(stat)(path);
        return stats.isDirectory();
    } catch (error) {
        return false; // If there's an error, the directory does not exist
    }
}

export async function createDirectory(path: string): Promise<void> {
    await promisify(mkdir)(path, { recursive: true });
}