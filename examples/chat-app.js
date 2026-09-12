import { SysStone } from '../js/index.js';

async function main() {
  const db = new SysStone('./chat-data');
  await db.open();

  const mensagens = db.collection('mensagens');

  await mensagens.insert({
    _id: 'm1',
    de: 'joao',
    para: 'maria',
    texto: 'Oi, tudo bem?',
    timestamp: Date.now(),
  });

  await mensagens.insert({
    _id: 'm2',
    de: 'maria',
    para: 'joao',
    texto: 'Tudo sim! E você?',
    timestamp: Date.now(),
  });

  const paraMaria = await mensagens.find({ para: 'maria' }).exec();
  console.log('Mensagens para Maria:', paraMaria.length);

  const doJoao = await mensagens.find({ de: 'joao' }).exec();
  console.log('Mensagens do João:', doJoao.map((m) => m.texto));

  await db.close();
}

main().catch(console.error);