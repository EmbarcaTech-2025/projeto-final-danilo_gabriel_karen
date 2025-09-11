// src/mqtt/handlers/gpsHandler.ts
import { GpsUsuario } from "../../models/gpsUsuarioModel";

export async function handleGpsMessage(packet: any, client: any) {
    try {
        const payload = JSON.parse(packet.payload.toString());
        const { usuarioId, latitude, longitude } = payload;

        await GpsUsuario.upsert({
            usuarioId,
            latitude,
            longitude,
            timestamp: new Date()
        });

        console.log(`Localização do usuário ${usuarioId} salva com sucesso!`);
    } catch (error) {
        console.error('Erro ao salvar localização:', error);
    }
}
