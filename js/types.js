// js/types.js — Tipos e utilidades compartilhadas do SysStone-DB
// Aqui ficam helpers que o resto do código JavaScript pode reutilizar.

// Gera um ID único simples baseado em tempo + aleatório
// Depois vamos trocar por algo mais robusto (ULID ou similar)
export function generateId() {
  const timestamp = Date.now().toString(36);
  const random = Math.random().toString(36).slice(2, 10);
  return `${timestamp}-${random}`;
}

// Valida se um valor é um objeto simples (não null, não array)
export function isPlainObject(value) {
  return (
    value !== null &&
    typeof value === 'object' &&
    !Array.isArray(value)
  );
}

// Converte uma string JSON em objeto, com segurança
export function safeParse(json) {
  try {
    return JSON.parse(json);
  } catch (err) {
    return null;
  }
}

// Converte um objeto em string JSON, com segurança
export function safeStringify(obj) {
  try {
    return JSON.stringify(obj);
  } catch (err) {
    return null;
  }
}

// Constantes do projeto
export const VERSION = '0.0.1';
export const ENGINE_NAME = 'sysstone';