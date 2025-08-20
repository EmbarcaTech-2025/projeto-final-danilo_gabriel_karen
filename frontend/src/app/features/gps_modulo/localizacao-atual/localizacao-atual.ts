import { Component, OnInit, AfterViewInit, Inject, PLATFORM_ID, signal, OnDestroy } from '@angular/core';
import { CommonModule, isPlatformBrowser } from '@angular/common';
import { GpsService } from '../gps';

@Component({
  selector: 'app-localizacao-atual',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="absolute inset-0 overflow-hidden">
      <div id="map-realtime" class="h-full w-full"></div>
      
      <div class="pointer-events-none absolute top-4 right-4 flex flex-col gap-2 z-999">
        <div class="pointer-events-auto bg-white/90 backdrop-blur border border-black/10 shadow-lg rounded-md px-3 py-2 text-xs text-black">
          <div class="font-medium mb-1">Localização Atual</div>
          <div class="text-xs text-black/70">
            @if (currentLocation()) {
              <div>Lat: {{ currentLocation()?.lat | number:'1.6-6' }}</div>
              <div>Lng: {{ currentLocation()?.lng | number:'1.6-6' }}</div>
              <div class="text-green-600">● Ativo</div>
            } @else {
              <div class="text-red-600">● Inativo</div>
            }
          </div>
        </div>
        
        <div class="pointer-events-auto bg-white/90 backdrop-blur border border-black/10 shadow-lg rounded-md p-2 flex items-center gap-2">
          <button class="px-2 py-1 text-xs rounded border border-black/20 hover:bg-black/5" (click)="toggleTracking()">
            {{ isTracking() ? 'Parar' : 'Iniciar' }} Rastreamento
          </button>
          <button class="px-2 py-1 text-xs rounded border border-black/20 hover:bg-black/5" (click)="centerOnLocation()">
            Centralizar
          </button>
        </div>
      </div>
    </div>
  `,
  styleUrl: './localizacao-atual.css'
})
export class LocalizacaoAtualComponent implements OnInit, AfterViewInit, OnDestroy {
  private map!: any;
  private L!: any;
  private locationMarker: any;
  private trackingInterval: any;
  
  readonly currentLocation = signal<{ lat: number; lng: number } | null>(null);
  readonly isTracking = signal(false);

  constructor(
    @Inject(PLATFORM_ID) private platformId: Object,
    private gpsService: GpsService
  ) {}

  ngOnInit(): void {
    // Inicializar rastreamento se disponível
    if (isPlatformBrowser(this.platformId) && 'geolocation' in navigator) {
      this.startTracking();
    }
  }

  async ngAfterViewInit() {
    if (isPlatformBrowser(this.platformId)) {
      this.L = await import('leaflet');
      this.initMap(this.L);
    }
  }

  ngOnDestroy(): void {
    this.stopTracking();
  }

  private initMap(L: any) {
    const baseMapUrl = 'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png';

    this.map = L.map('map-realtime', {
      center: [-15.905575916705892, -48.067376693952035],
      zoom: 15
    });

    L.tileLayer(baseMapUrl, {
      attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(this.map);

    // Criar marcador de localização
    this.locationMarker = L.marker([0, 0], {
      icon: L.divIcon({
        className: 'custom-marker',
        html: '<div class="w-4 h-4 bg-blue-600 rounded-full border-2 border-white shadow-lg"></div>',
        iconSize: [16, 16]
      })
    });
  }

  startTracking(): void {
    if (!isPlatformBrowser(this.platformId) || !('geolocation' in navigator)) {
      alert('Geolocalização não disponível neste dispositivo.');
      return;
    }

    this.isTracking.set(true);
    
    // Obter posição inicial
    navigator.geolocation.getCurrentPosition(
      (position) => this.updateLocation(position),
      (error) => console.error('Erro ao obter localização:', error),
      { enableHighAccuracy: true }
    );

    // Monitorar mudanças de posição
    this.trackingInterval = navigator.geolocation.watchPosition(
      (position) => this.updateLocation(position),
      (error) => console.error('Erro ao monitorar localização:', error),
      { enableHighAccuracy: true, maximumAge: 30000, timeout: 27000 }
    );
  }

  stopTracking(): void {
    this.isTracking.set(false);
    if (this.trackingInterval) {
      navigator.geolocation.clearWatch(this.trackingInterval);
      this.trackingInterval = null;
    }
  }

  toggleTracking(): void {
    if (this.isTracking()) {
      this.stopTracking();
    } else {
      this.startTracking();
    }
  }

  private updateLocation(position: GeolocationPosition): void {
    const lat = position.coords.latitude;
    const lng = position.coords.longitude;
    
    this.currentLocation.set({ lat, lng });
    
    if (this.locationMarker && this.map) {
      const latlng = this.L.latLng(lat, lng);
      this.locationMarker.setLatLng(latlng);
      
      if (!this.map.hasLayer(this.locationMarker)) {
        this.locationMarker.addTo(this.map);
      }
    }
  }

  centerOnLocation(): void {
    if (this.currentLocation() && this.map) {
      this.map.setView([this.currentLocation()!.lat, this.currentLocation()!.lng], 16);
    }
  }
}
