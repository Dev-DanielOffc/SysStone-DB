import { SysStone } from '../js/index.js';

async function main() {
  const db = new SysStone('./example-data');
  await db.open();

  const users = db.collection('usuarios');

  await users.insert({ _id: 'u1', nome: 'João', idade: 25, cidade: 'SP' });
  await users.insert({ _id: 'u2', nome: 'Maria', idade: 30, cidade: 'RJ' });
  await users.insert({ _id: 'u3', nome: 'Carlos', idade: 17, cidade: 'SP' });

  console.log('Total de usuários:', await users.count());

  const adultos = await users.find({ idade: { $gte: 18 } }).exec();
  console.log('Adultos:', adultos.map((u) => u.nome));

  const paulistas = await users.find({ cidade: 'SP' }).exec();
  console.log('De SP:', paulistas.map((u) => u.nome));

  await db.close();
}

main().catch(console.error);