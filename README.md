# SysStone-DB 🪨

> Base de datos embebida, asíncrona y optimizada para aplicaciones Node.js modernas.

## Qué es

SysStone-DB es una base de datos **embebida** (como SQLite), pero enfocada en:

- ⚡ **Alta performance** — core en C++ compilado con N-API
- 🔄 **API asíncrona nativa** — `async/await` desde el primer día
- 📦 **Almacenamiento segmentado** — un directorio, no un archivo único
- 🔍 **Queries con filtros** — estilo MongoDB (`$gt`, `$lt`, `$contains`)
- 🔐 **Cifrado por segmento** — planeado para apps de mensajería
- 🌐 **Sincronización P2P** — planeado para múltiples dispositivos

## Filosofía

A diferencia de PostgreSQL, MySQL o MongoDB, **SysStone-DB no necesita servidor**.

Nada de VPS, contraseñas, puertos ni configuración. Es solo una carpeta en el disco.

```bash
import { SysStone } from 'sysstone-db';

const db = new SysStone('./data');
await db.open();

const usuarios = db.collection('usuarios');
await usuarios.insert({ nombre: 'Juan', edad: 20 });

const adultos = await usuarios.find({ edad: { $gte: 18 } }).exec();
console.log(adultos);
```

## Instalación

```bash
npm install sysstone-db
```

## Uso básico

### Abrir y cerrar

```bash
import { SysStone } from 'sysstone-db';

const db = new SysStone('./mi-base');
await db.open();
// ... usas la base
await db.close();
```

### Insertar documentos

```bash
const usuarios = db.collection('usuarios');

await usuarios.insert({ _id: 'u1', nombre: 'Juan', edad: 25 });
await usuarios.insert({ _id: 'u2', nombre: 'María', edad: 30 });
```

### Buscar por ID

```bash
const user = await usuarios.findById('u1');
console.log(user.nombre); // Juan
```

### Buscar con filtros

```bash
const adultos = await usuarios.find({ edad: { $gte: 18 } }).exec();
const deMadrid = await usuarios.find({ ciudad: 'Madrid' }).exec();
const gomezes = await usuarios.find({ nombre: { $contains: 'Gomez' } }).exec();
```

### Contar documentos

```bash
const total = await usuarios.count();
```

### Eliminar

```bash
await usuarios.remove('u1');
```

### Listar claves

```bash
const keys = await usuarios.keys();
```

## Operadores soportados

| Operador | Descripción |
|---|---|
| `$eq` | Igual |
| `$ne` | Diferente |
| `$gt` | Mayor que |
| `$gte` | Mayor o igual |
| `$lt` | Menor que |
| `$lte` | Menor o igual |
| `$contains` | Contiene substring |

## Estructura de almacenamiento

Cada colección se convierte en un directorio, y dentro de ella viven segmentos append-only:

```bash
mi-base/
├── usuarios/
│   ├── segment-00000.log
│   └── segment-00001.log
└── mensajes/
    └── segment-00000.log
```

Cuando un segmento supera 1 MB, se crea uno nuevo automáticamente.

## Performance

Para correr los benchmarks:

```bash
npm install
npm run build
npm run bench
```

Para comparar con SQLite:

```bash
npm install better-sqlite3 --no-save
node benchmarks/vs-sqlite.js
```

## Desarrollo

### Requisitos

- Node.js >= 18
- Compilador C++ (GCC, Clang o MSVC)
- Python 3 (para node-gyp)

### Comandos

```bash
npm install       # instala dependencias
npm run build     # compila el core en C++
npm run rebuild   # recompila desde cero
npm run clean     # limpia el build
npm test          # corre los tests
npm run bench     # corre el benchmark
npm run lint      # corre el lint
```

### Estructura del proyecto

```bash
sysstone-db/
├── core/               # Motor en C++
│   ├── include/        # Headers (.h)
│   └── src/            # Implementaciones (.cpp)
├── bindings/           # Puente N-API entre C++ y Node
│   ├── include/
│   └── src/
├── js/                 # API pública en JavaScript
├── tests/              # Tests con node:test
├── benchmarks/         # Benchmarks
├── examples/           # Ejemplos ejecutables
├── binding.gyp         # Configuración de node-gyp
└── package.json
```

## Inspiraciones

- **SQLite** — almacenamiento embebido y B-tree
- **PostgreSQL** — MVCC y replicación por WAL
- **RocksDB** — almacenamiento segmentado (LSM)
- **MongoDB** — API orientada a documentos

Ningún código fue copiado. Solo se estudiaron ideas arquitectónicas públicas.

## Roadmap

- [x] Fase 0 — Estructura inicial y setup
- [x] Fase 1 — Pager, persistencia en segmentos, queries básicas
- [ ] Fase 2 — WAL propio y MVCC
- [ ] Fase 3 — Índices secundarios y compaction
- [ ] Fase 4 — Cifrado por segmento
- [ ] Fase 5 — Sincronización P2P
- [ ] Fase 6 — VM de bytecode para queries

## Licencia

MIT — ver el archivo [LICENSE](./LICENSE).