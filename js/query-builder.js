import { safeParse } from './types.js';

const OPERATORS = {
  $eq: '$eq',
  $ne: '$ne',
  $gt: '$gt',
  $gte: '$gte',
  $lt: '$lt',
  $lte: '$lte',
  $contains: '$contains',
};

export class QueryBuilder {
  constructor(db, collection, filter) {
    this._db = db;
    this._collection = collection;
    this._filter = filter;
  }

  async exec() {
    const filters = this._buildFilters(this._filter);
    const raw = this._db.find(this._collection, JSON.stringify(filters));
    return raw.map((doc) => safeParse(doc)).filter(Boolean);
  }

  _buildFilters(filter) {
    const filters = [];

    for (const [field, condition] of Object.entries(filter)) {
      if (typeof condition === 'object' && condition !== null) {
        for (const [op, value] of Object.entries(condition)) {
          filters.push({
            field,
            op: OPERATORS[op] || '$eq',
            value: String(value),
          });
        }
      } else {
        filters.push({
          field,
          op: '$eq',
          value: String(condition),
        });
      }
    }

    return filters;
  }

  then(resolve, reject) {
    return this.exec().then(resolve, reject);
  }
}