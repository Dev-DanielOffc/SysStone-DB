import { createRequire } from 'module';
import { generateId, safeParse, safeStringify } from './types.js';
import { QueryBuilder } from './query-builder.js';

const require = createRequire(import.meta.url);
const binding = require('../build/Release/sysstone.node');

export class SysStone {
  constructor(path) {
    if (!path || typeof path !== 'string') {
      throw new Error('SysStone: caminho do banco é obrigatório');
    }
    this._db = new binding.Database(path);
    this._path = path;
    this._isOpen = false;
  }

  async open() {
    const ok = this._db.open();
    this._isOpen = ok;
    return ok;
  }

  async close() {
    const ok = this._db.close();
    this._isOpen = false;
    return ok;
  }

  collection(name) {
    if (!name || typeof name !== 'string') {
      throw new Error('SysStone: nome da coleção é obrigatório');
    }
    return new Collection(this._db, name);
  }
}

export class Collection {
  constructor(db, name) {
    this._db = db;
    this._name = name;
  }

  async insert(doc) {
    if (!doc || typeof doc !== 'object') {
      throw new Error('SysStone: documento inválido');
    }

    const key = doc._id || generateId();
    const value = safeStringify(doc);

    if (!value) {
      throw new Error('SysStone: documento não é serializável');
    }

    this._db.put(this._name, key, value);
    return { ...doc, _id: key };
  }

  async findById(id) {
    const raw = this._db.get(this._name, id);
    if (!raw) return null;
    return safeParse(raw);
  }

  async remove(id) {
    return this._db.remove(this._name, id);
  }

  async keys() {
    return this._db.keys(this._name);
  }

  async count() {
    return this._db.count(this._name);
  }

  find(filter = {}) {
    return new QueryBuilder(this._db, this._name, filter);
  }
}

export default SysStone;