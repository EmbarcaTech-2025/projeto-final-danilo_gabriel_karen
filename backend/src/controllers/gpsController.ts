import { Request, Response } from 'express';
import * as sessionService from '../services/gpsService';


// exemplo de função controller
export const list = async (_req: Request, res: Response) => {
  try {
    res.json({});
  } catch (err) {
    res.status(500).json({ error: (err as Error).message });
  }
};