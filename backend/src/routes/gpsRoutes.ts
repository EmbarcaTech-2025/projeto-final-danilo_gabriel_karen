import { Router } from 'express';
import { adicionarAreaSeguraController } from '../controllers/gpsController';

const router = Router();

// Rota para adicionar uma nova área segura
router.post('/adicionar', adicionarAreaSeguraController);

export default router;