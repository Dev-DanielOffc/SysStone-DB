import { test } from 'node:test';
import assert from 'node:assert';
import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';

const TEST_PATH = join(process.cwd(), 'test-data', 'query');

if (existsSync(TEST_PATH)) {
  rmSync(TEST_PATH, { recursive: true, force: true });
}

test('busca com $gt', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const users = db.collection('users_gt');
  await users.insert({ _id: 'a', nome: 'Ana', idade: 15 });
  await users.insert({ _id: 'b', nome: 'Bia', idade: 22 });
  await users.insert({ _id: 'c', nome: 'Caio', idade: 30 });

  const result = await users.find({ idade: { $gt: 18 } }).exec();
  assert.strictEqual(result.length, 2);
  assert.ok(result.every((u) => u.idade > 18));

  await db.close();
});

test('busca com $lt', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const col = db.collection('users_lt');
  await col.insert({ _id: 'a', nome: 'Ana', idade: 15 });
  await col.insert({ _id: 'b', nome: 'Bia', idade: 22 });
  await col.insert({ _id: 'c', nome: 'Caio', idade: 30 });

  const result = await col.find({ idade: { $lt: 20 } }).exec();
  assert.strictEqual(result.length, 1);
  assert.strictEqual(result[0].nome, 'Ana');

  await db.close();
});

test('busca com igualdade direta', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const col = db.collection('users_eq');
  await col.insert({ _id: 'a', nome: 'Ana', cidade: 'SP' });
  await col.insert({ _id: 'b', nome: 'Bia', cidade: 'RJ' });

  const result = await col.find({ cidade: 'SP' }).exec();
  assert.strictEqual(result.length, 1);
  assert.strictEqual(result[0].nome, 'Ana');

  await db.close();
});

test('busca com $contains', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const col = db.collection('users_contains');
  await col.insert({ _id: 'a', nome: 'Ana Silva' });
  await col.insert({ _id: 'b', nome: 'Bia Souza' });
  await col.insert({ _id: 'c', nome: 'Carlos Silva' });

  const result = await col.find({ nome: { $contains: 'Silva' } }).exec();
  assert.strictEqual(result.length, 2);

  await db.close();
});

test('retorna vazio quando nenhum bate', async () => {
  const db = new SysStone(TEST_PATH);
  await db.open();

  const col = db.collection('users_empty');
  await col.insert({ _id: 'a', idade: 10 });

  const result = await col.find({ idade: { $gt: 100 } }).exec();
  assert.strictEqual(result.length, 0);

  await db.close();
});