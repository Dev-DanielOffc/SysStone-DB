import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';
import { performance } from 'node:perf_hooks';

const BENCH_PATH = join(process.cwd(), 'test-data', 'vs-sqlite');
const TOTAL = 10000;

if (existsSync(BENCH_PATH)) {
  rmSync(BENCH_PATH, { recursive: true, force: true });
}

function fmt(n) {
  return n.toLocaleString('pt-BR');
}

async function bench(name, fn) {
  const start = performance.now();
  await fn();
  const end = performance.now();
  const ms = end - start;
  console.log(`${name.padEnd(30)} ${ms.toFixed(2).padStart(10)} ms  ${fmt(Math.round((TOTAL / ms) * 1000)).padStart(15)} ops/s`);
}

async function runSysStone() {
  console.log('\n🪨 SysStone-DB');
  console.log('─'.repeat(70));

  const db = new SysStone(BENCH_PATH);
  await db.open();
  const col = db.collection('bench');

  await bench('insert', async () => {
    for (let i = 0; i < TOTAL; i++) {
      await col.insert({ _id: `id-${i}`, valor: i, nome: `User ${i}` });
    }
  });

  await bench('findById', async () => {
    for (let i = 0; i < TOTAL; i++) {
      await col.findById(`id-${i}`);
    }
  });

  await bench('query $gt', async () => {
    await col.find({ valor: { $gt: TOTAL / 2 } }).exec();
  });

  await db.close();
}

async function runSQLite() {
  console.log('\n📦 SQLite (better-sqlite3)');
  console.log('─'.repeat(70));

  let Database;
  try {
    Database = (await import('better-sqlite3')).default;
  } catch {
    console.log('⚠️  better-sqlite3 não instalado. Rode:');
    console.log('    npm install better-sqlite3 --no-save');
    return;
  }

  const dbPath = join(BENCH_PATH, 'sqlite.db');
  const db = new Database(dbPath);

  db.exec('CREATE TABLE IF NOT EXISTS bench (id TEXT PRIMARY KEY, valor INTEGER, nome TEXT)');
  db.exec('DELETE FROM bench');

  const insertStmt = db.prepare('INSERT INTO bench (id, valor, nome) VALUES (?, ?, ?)');
  const getStmt = db.prepare('SELECT * FROM bench WHERE id = ?');

  await bench('insert', async () => {
    const tx = db.transaction(() => {
      for (let i = 0; i < TOTAL; i++) {
        insertStmt.run(`id-${i}`, i, `User ${i}`);
      }
    });
    tx();
  });

  await bench('findById', async () => {
    for (let i = 0; i < TOTAL; i++) {
      getStmt.get(`id-${i}`);
    }
  });

  await bench('query WHERE valor >', async () => {
    db.prepare('SELECT * FROM bench WHERE valor > ?').all(TOTAL / 2);
  });

  db.close();
}

async function main() {
  console.log('\n⚔️  SysStone-DB vs SQLite');
  console.log('═'.repeat(70));
  console.log(`Operações: ${fmt(TOTAL)}`);

  await runSysStone();
  await runSQLite();

  console.log('\n✅ Benchmark finalizado.\n');
}

main().catch((err) => {
  console.error('❌ Erro:', err);
  process.exit(1);
});