import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class GpsService {

  constructor(private http: HttpClient) { }

  getDadosGps(): Observable<any> {
    // Exemplo de chamada para uma API
    return this.http.get('https://api.exemplo.com/gps');
  }
}