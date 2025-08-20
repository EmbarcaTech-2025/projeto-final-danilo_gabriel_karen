import { Routes } from '@angular/router';

export const routes: Routes = [
  {
    path: 'gps',
    loadChildren: () =>
      import('./features/gps_modulo/gps_modulo.routes').then(m => m.GPS_ROUTES),
  },
  {
    path: '',
    redirectTo: 'gps',
    pathMatch: 'full'
  },
  
];
