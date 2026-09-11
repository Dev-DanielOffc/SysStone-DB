// benchmarks/run.js — Benchmark do SysStone-DB
// Mede quantas operações por segundo conseguimos fazer.
// Roda com: npm run bench

import { SysStone } from '../js/index.js';
import { rmSync, existsSync } from 'node:fs';
import { join } from 'node:path';
import { performance } from 'node:perf_hooks';

const BENCH_PATH = join(process.cwd(), 'test-data', 'bench');

// Limpa bench anterior
if (existsSync(BENCH_PATH)) {
  rmSync(BENCH_PATH, { recursive: true, force: true });
}

// Utilitário pra formatar números
function fmt(n) {
  return n.toLocaleString('pt-BR');
}

// Roda um benchmark e mostra o resultado
async function bench(name, fn) {
  const start = performance.now();
  const ops = await fn();
  const end = performance.now();
  const ms = end - start;
  const opsPerSec = (ops / ms) * 1000;

  console.log(`\n▶ ${name}`);
  console.log(`   Operações : ${fmt(ops)}`);
  console.log(`   Tempo     : ${ms.toFixed(2)} ms`);
  console.log(`   Ops/seg   : ${fmt(Math.round(opsPerSec))}`);
}

// ============================
// Benchmark 1 — Inserções
// ============================
async function benchInserts() {
  const db = new SysStone(BENCH_PATH);
  await db.open();

  const col = db.collection('bench_insert');
  const TOTAL = 10000;

  for (let i = 0; i < TOTAL; i++) {
    await col.insert({
      _id: `id-${i}`,
      valor: i,
      nome: `Usuario ${i}`,
    });
  }

  await db.close();
  return TOTAL;
}

// ============================
// Benchmark 2 — Leituras
// ============================
async function benchReads() {
  const db = new SysStone(BENCH_PATH);
  await db.open();

  const col = db.collection('bench_insert');
  const TOTAL = 10000;
  let found = 0;

  for (let i = 0; i < TOTAL; i++) {
    const doc = await col.findById(`id-${i}`);
    if (doc) found++;
  }

  await db.close();
  return found;
}

// ============================
// Benchmark 3 — Remoções
// ============================
async function benchRemoves() {
  const db = new SysStone(BENCH_PATH);
  await db.open();

  const col = db.collection('bench_insert');
  const TOTAL = 5000;
  let removed = 0;

  for (let i = 0; i < TOTAL; i++) {
    const ok = await col.remove(`id-${i}`);
    if (ok) removed++;
  }

  await db.close();
  return removed;
}

// ============================
// Main
// ============================
async function main() {
  console.log('🪨 SysStone-DB — Benchmark');
  console.log('================================');

  await bench('Inserções (10.000)', benchInserts);
  await bench('Leituras (10.000)', benchReads);
  await bench('Remoções (5.000)', benchRemoves);

  console.log('\n✅ Benchmark finalizado.');
}

main().catch((err) => {
  console.error('❌ Erro no benchmark:', err);
  process.exit(1);
});