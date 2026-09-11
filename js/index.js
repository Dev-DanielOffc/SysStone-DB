// js/index.js — API pública do SysStone-DB
// Aqui o usuário final interage com o banco. Nada de C++ aqui.

// Importa o módulo nativo compilado em C++
import { createRequire } from 'module';
const require = createRequire(import.meta.url);

// Carrega o binding compilado (vira sysstone.node depois do build)
const binding = require('../build/Release/sysstone.node');

// Classe principal que o usuário vai usar
export class SysStone {
  // Construtor recebe o caminho da pasta onde o banco vai ficar
  constructor(path) {
    if (!path || typeof path !== 'string') {
      throw new Error('SysStone: caminho do banco é obrigatório');
    }

    // Cria a instância nativa do banco
    this._db = new binding.Database(path);
    this._path = path;
    this._isOpen = false;
  }

  // Abre o banco
  async open() {
    const ok = this._db.open();
    this._isOpen = ok;
    return ok;
  }

  // Fecha o banco
  async close() {
    const ok = this._db.close();
    this._isOpen = false;
    return ok;
  }

  // Retorna uma "coleção" (como uma tabela no SQL)
  collection(name) {
    if (!name || typeof name !== 'string') {
      throw new Error('SysStone: nome da coleção é obrigatório');
    }
    return new Collection(this._db, name);
  }
}

// Representa uma coleção de documentos
export class Collection {
  constructor(db, name) {
    this._db = db;
    this._name = name;
  }

  // Insere um documento na coleção
  // O documento precisa ter um campo "_id" ou usamos a chave informada
  async insert(doc) {
    if (!doc || typeof doc !== 'object') {
      throw new Error('SysStone: documento inválido');
    }

    // Usa "_id" como chave ou gera uma aleatória
    const key = doc._id || this._generateId();

    // Serializa o documento em JSON (por enquanto)
    const value = JSON.stringify(doc);

    this._db.put(this._name, key, value);
    return { ...doc, _id: key };
  }

  // Busca um documento pela chave
  async findById(id) {
    const raw = this._db.get(this._name, id);
    if (!raw) return null;
    return JSON.parse(raw);
  }

  // Remove um documento
  async remove(id) {
    return this._db.remove(this._name, id);
  }

  // Lista todas as chaves da coleção
  async keys() {
    return this._db.keys(this._name);
  }

  // Gera um ID simples (depois vira algo mais robusto)
  _generateId() {
    return Date.now().toString(36) + Math.random().toString(36).slice(2, 8);
  }
}

// Exportação padrão
export default SysStone;