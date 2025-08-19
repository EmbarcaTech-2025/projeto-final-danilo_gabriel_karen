import { Request, Response } from 'express';
import * as gpsAreaService from '../services/gpsService';


export async function adicionarAreaSeguraController(req: Request, res: Response) {
    try {
        const { pontos } = req.body;

        if (!pontos || !Array.isArray(pontos) || pontos.length < 3) {
            return res.status(400).json({ message: 'Dados de pontos inválidos.' });
        }

        const novaArea = await gpsAreaService.adicionarAreaSegura(pontos);

        res.status(201).json({
            message: 'Área segura adicionada com sucesso!',
            data: novaArea
        });

    } catch (error) {
        res.status(500)
        //.json({ message: 'Erro interno no servidor.', error: error.message });
    }
}