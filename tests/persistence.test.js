import { test } from 'node:test';
import assert from 'node:assert';
import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';

const TEST_PATH = join(process.cwd(), 'test-data', 'persistence');

if (existsSync(TEST_PATH)) {
  rmSync(TEST_PATH, { recursive: true, force: true });
}

test('dados sobrevivem ao fechar e reabrir', async () => {
  const db1 = new SysStone(TEST_PATH);
  await db1.open();

  const col1 = db1.collection('persist');
  await col1.insert({ _id: 'k1', valor: 'guardado' });
  await col1.insert({ _id: 'k2', valor: 'também' });

  await db1.close();

  const db2 = new SysStone(TEST_PATH);
  await db2.open();

  const col2 = db2.collection('persist');
  const doc1 = await col2.findById('k1');
  const doc2 = await col2.findById('k2');

  assert.strictEqual(doc1.valor, 'guardado');
  assert.strictEqual(doc2.valor, 'também');
  assert.strictEqual(await col2.count(), 2);

  await db2.close();
});

test('remoção persiste após reabrir', async () => {
  const db1 = new SysStone(TEST_PATH);
  await db1.open();

  const col1 = db1.collection('persist_del');
  await col1.insert({ _id: 'x', valor: 1 });
  await col1.insert({ _id: 'y', valor: 2 });
  await col1.remove('x');

  await db1.close();

  const db2 = new SysStone(TEST_PATH);
  await db2.open();

  const col2 = db2.collection('persist_del');
  assert.strictEqual(await col2.findById('x'), null);
  assert.strictEqual(await col2.findById('y') !== null, true);

  await db2.close();
});