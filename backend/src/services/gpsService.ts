import { Session } from '../models/gpsModel';

// exemplo de service
export const startSession = async (category: string, notes?: string) => {
  return await Session.create({
    category,
    notes,
    startTime: new Date()
  });
};