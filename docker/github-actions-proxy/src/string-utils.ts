export function isNullOrEmpty(value: string | null | undefined): boolean {
    return value === null || value === undefined || value.trim() === '';
}