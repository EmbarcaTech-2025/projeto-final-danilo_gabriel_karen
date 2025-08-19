import { GpsAreaSegura } from '../models/gpsModel';

export async function adicionarAreaSegura(pontos: any[]) {
    try {
        // Desativa todas as áreas seguras ativas
        await GpsAreaSegura.update({ ativo: false }, { where: { ativo: true } });

        const novaArea = await GpsAreaSegura.create({
            pontos: pontos,
            ativo: true
        });

        return novaArea;
    } catch (error) {
        console.error('Erro ao adicionar área segura:', error);
        throw new Error('Falha ao adicionar a área segura.');
    }
}