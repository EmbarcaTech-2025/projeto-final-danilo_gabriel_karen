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

  saveSafeArea(payload: { nome: string; pontos: { lat: number; lng: number }[] }): Observable<any> {
    // Substitua pela URL real do seu backend
    return this.http.post('/api/areas-seguras', payload);
  }

  listarAreas(): Observable<any[]> {
    return this.http.get<any[]>('/api/areas-seguras');
  }

  ativarArea(id: string): Observable<void> {
    return this.http.put<void>(`/api/areas-seguras/${id}/ativar`, {});
  }

  apagarArea(id: string): Observable<void> {
    return this.http.delete<void>(`/api/areas-seguras/${id}`);
  }
}