import { test } from 'node:test';
import assert from 'node:assert';
import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';

const TEST_PATH = join(process.cwd(), 'test-data', 'basic');

if (existsSync(TEST_PATH)) {
  rmSync(TEST_PATH, { recursive: true, force: true });
}

test('abre e fecha o banco', async () => {
  const db = new SysStone(TEST_PATH);
  assert.strictEqual(await db.open(), true);
  assert.strictEqual(await db.close(), true);
});

test('insere e busca um documento', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const users = db.collection('usuarios');
  await users.insert({ _id: 'u1', nome: 'João', idade: 20 });

  const found = await users.findById('u1');
  assert.strictEqual(found.nome, 'João');
  assert.strictEqual(found.idade, 20);

  await db.close();
});

test('remove um documento', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const users = db.collection('usuarios');
  await users.insert({ _id: 'u2', nome: 'Maria' });
  assert.strictEqual(await users.remove('u2'), true);
  assert.strictEqual(await users.findById('u2'), null);

  await db.close();
});

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

test('conta documentos', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const col = db.collection('contagem');
  await col.insert({ _id: 'x' });
  await col.insert({ _id: 'y' });
  await col.insert({ _id: 'z' });

  assert.strictEqual(await col.count(), 3);

  await db.close();
});