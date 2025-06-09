# Webway Monorepo

A TypeScript monorepo containing packages for Redpanda cluster management and binary data parsing.

## Structure

```
webway/
├── packages/
│   ├── redpanda-cluster/    # Redpanda cluster management
│   └── parser/              # Binary data parser and generator
├── package.json             # Root package.json with workspaces
└── README.md
```

## Packages

### @webway/redpanda-cluster

A package for managing Redpanda clusters with Kafka-compatible operations.

**Features:**
- Cluster connection management
- Producer and consumer creation
- Topic management
- Admin operations
- **Kubernetes deployment manifests and scripts**
- Ready-to-use StatefulSet for production deployments

### @webway/parser

A package for parsing binary data and generating test data.

**Features:**
- Schema-based binary data parsing
- Data serialization/deserialization
- Test data generation with Faker.js
- Support for various data types (integers, floats, strings, booleans)

## Setup

1. Install dependencies:
```bash
yarn install
```

2. Build all packages:
```bash
yarn build
```

3. Run development mode:
```bash
yarn dev
```

## Usage Examples

### Redpanda Cluster

#### TypeScript API

```typescript
import { RedpandaCluster } from '@webway/redpanda-cluster';

const cluster = new RedpandaCluster({
  brokers: ['localhost:9092'],
  clientId: 'my-app'
});

// Create a topic
await cluster.createTopic('my-topic', 3, 1);

// Get producer
const producer = await cluster.createProducer();
await producer.connect();

// Send message
await producer.send({
  topic: 'my-topic',
  messages: [{ value: 'Hello Redpanda!' }]
});
```

#### Kubernetes Deployment

Deploy a production-ready Redpanda cluster to Kubernetes:

```bash
# Navigate to the redpanda-cluster package
cd packages/redpanda-cluster

# Deploy the cluster
yarn k8s:deploy

# Check status
yarn k8s:status

# Clean up when done
yarn k8s:cleanup
```

The deployment includes:
- 3-node Redpanda cluster with persistent storage
- Internal and external services
- ConfigMap with optimized settings
- Health checks and monitoring

See [packages/redpanda-cluster/k8s/README.md](packages/redpanda-cluster/k8s/README.md) for detailed instructions.

### Binary Parser

```typescript
import { BinaryParser, DataGenerator } from '@webway/parser';

const schema = {
  fields: [
    { name: 'id', type: 'uint32' },
    { name: 'temperature', type: 'float' },
    { name: 'name', type: 'string', length: 20 },
    { name: 'active', type: 'boolean' }
  ]
};

// Generate test data
const generator = new DataGenerator(schema);
const testData = generator.generate();

// Parse binary data
const parser = new BinaryParser(schema);
const binaryData = parser.serialize(testData);
const parsedData = parser.parse(binaryData);
```

## Development

### Scripts

- `yarn build` - Build all packages
- `yarn dev` - Start development mode for all packages
- `yarn test` - Run tests for all packages
- `yarn lint` - Lint all packages
- `yarn clean` - Clean build artifacts

### Adding New Packages

1. Create a new directory in `packages/`
2. Add a `package.json` with name `@webway/package-name`
3. Add TypeScript configuration
4. The package will be automatically included in workspace operations

## Environment Variables

### Redpanda Cluster

Create a `.env` file in the root or in the `packages/redpanda-cluster` directory:

```env
REDPANDA_BROKERS=localhost:9092
REDPANDA_CLIENT_ID=webway-app
REDPANDA_SSL=false
```

## License

Private - All rights reserved 