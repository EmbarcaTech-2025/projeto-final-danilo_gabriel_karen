import { Router } from 'express';

import {
    adicionarAreaSeguraController,
    verificarAreaSeguraController,
    atualizarAreaAtivaController,
    deletarAreaController,
    listarAreasController
} from '../controllers/gpsController'; 

const router = Router();

// Adicionar uma nova área segura
router.post('/adicionar', adicionarAreaSeguraController);

// Verificar se um ponto está dentro da área segura ativa
router.post('/verificar', verificarAreaSeguraController);

// Atualizar qual área está ativa (ativa a área pelo id)
router.put('/ativar/:id', atualizarAreaAtivaController);

// Deletar uma área pelo id
router.delete('/deletar/:id', deletarAreaController);

router.get('/listar', listarAreasController);

export default router;