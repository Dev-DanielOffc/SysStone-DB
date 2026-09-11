// tests/basic.test.js — Testes básicos do SysStone-DB
// Rodam com "npm test" usando o test runner nativo do Node.

import { test } from 'node:test';
import assert from 'node:assert';
import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';

// Pasta temporária para os testes
const TEST_PATH = join(process.cwd(), 'test-data', 'basic');

// Antes de tudo, limpa a pasta de teste
if (existsSync(TEST_PATH)) {
  rmSync(TEST_PATH, { recursive: true, force: true });
}

// Teste 1 — abre e fecha o banco
test('abre e fecha o banco corretamente', async () => {
  const db = new SysStone(TEST_PATH);
  const opened = await db.open();
  assert.strictEqual(opened, true);

  const closed = await db.close();
  assert.strictEqual(closed, true);
});

// Teste 2 — insere e busca um documento
test('insere e busca um documento', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const users = db.collection('usuarios');
  const inserted = await users.insert({ _id: 'u1', nome: 'João', idade: 20 });

  assert.strictEqual(inserted._id, 'u1');

  const found = await users.findById('u1');
  assert.strictEqual(found.nome, 'João');
  assert.strictEqual(found.idade, 20);

  await db.close();
});

// Teste 3 — remove um documento
test('remove um documento', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const users = db.collection('usuarios');
  await users.insert({ _id: 'u2', nome: 'Maria' });

  const removed = await users.remove('u2');
  assert.strictEqual(removed, true);

  const found = await users.findById('u2');
  assert.strictEqual(found, null);

  await db.close();
});

// Teste 4 — lista chaves da coleção
test('lista chaves da coleção', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const items = db.collection('itens');
  await items.insert({ _id: 'a', valor: 1 });
  await items.insert({ _id: 'b', valor: 2 });

  const keys = await items.keys();
  assert.ok(keys.includes('a'));
  assert.ok(keys.includes('b'));

  await db.close();
});