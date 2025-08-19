import { Routes } from '@angular/router';
import { VisualizacaoGps } from './visualizacao-gps/visualizacao-gps';

export const GPS_ROUTES: Routes = [
  {
    path: '',
    component: VisualizacaoGps,
  },
  // Adicione aqui outras rotas filhas, se necessário (ex: :id, etc)
];