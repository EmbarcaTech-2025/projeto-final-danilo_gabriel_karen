// src/server.ts
import express from 'express';
import bodyParser from 'body-parser';
import cors from 'cors';

// Rotas importadas
import gpsAreaRoutes from './routes/gpsRoutes';

const app = express();
const port = 3000;

app.use(cors());
app.use(bodyParser.json());

// Rota de teste
app.get('/', (req, res) => {
    res.send('Servidor Node.js com TypeScript rodando!');
});

// Rotas API
app.use('/api/gps_area_segura', gpsAreaRoutes);

// Inicia o servidor
app.listen(port, () => {
    console.log(`Servidor rodando em http://localhost:${port}`);
});

export default app;