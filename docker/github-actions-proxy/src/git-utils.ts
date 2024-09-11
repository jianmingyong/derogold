import { spawn } from 'child_process';

export function cloneRepository(repoUrl: string, destinationPath: string): Promise<void> {
    return new Promise((resolve, reject) => {
        const process = spawn('git', ['clone', repoUrl, destinationPath], {
            stdio: 'inherit', // Inherit the parent process's stdio streams
        });

        process.on('error', (error) => {
            reject(error);
        });

        process.on('exit', (code) => {
            if (code === 0) {
                resolve();
            } else {
                reject(new Error(`Git clone failed with exit code ${code}`));
            }
        });
    });
}

export function checkoutRepository(ref: string, repoDirectory: string): Promise<void> {
    return new Promise((resolve, reject) => {
        const process = spawn('git', ['checkout', '-f', '--detach', ref], {
            stdio: 'inherit', // Inherit the parent process's stdio streams
            cwd: repoDirectory
        });

        process.on('error', (error) => {
            reject(error);
        });

        process.on('exit', (code) => {
            if (code === 0) {
                resolve();
            } else {
                reject(new Error(`Git checkout failed with exit code ${code}`));
            }
        });
    });
}