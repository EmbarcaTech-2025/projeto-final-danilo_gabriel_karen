import express from 'express';
import sessionRoutes from './routes/gpsRoutes';

const app = express();

app.use(express.json());
app.use('/sessions', sessionRoutes);

app.get('/teste', (req, res) => {
    res.send('Servidor está funcionando!');
})

export default app;