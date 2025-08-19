import { Component, OnInit, AfterViewInit, Inject, PLATFORM_ID } from '@angular/core';
import { CommonModule, isPlatformBrowser } from '@angular/common';
import { GpsService } from '../gps';

@Component({
  selector: 'app-visualizacao-gps',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './visualizacao-gps.html',
  styleUrl: './visualizacao-gps.css'
})
export class VisualizacaoGps implements OnInit, AfterViewInit {

  private map!: any; // Use 'any' para evitar erros de tipagem
  markers: any[] = [
    // Mantenha os dados do marcador
  ];

  constructor(
    @Inject(PLATFORM_ID) private platformId: Object
  ) { }

  ngOnInit(): void {

  }

  async ngAfterViewInit() {
    if (isPlatformBrowser(this.platformId)) {
      const L = await import('leaflet');
      this.initMap(L);
    }
  }

  private initMap(L: any) {
    const baseMapURl = 'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png';

    this.map = L.map('map', {
      center: [-15.905575916705892, -48.067376693952035],
      zoom: 15
    });
  
    L.tileLayer(baseMapURl, {
        attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(this.map);
  
    this.map.on('load', () => {
      this.centerMap(L);
    });
  }
  

  private centerMap(L: any) {
    if (this.markers.length > 0) {
      if (this.markers.length === 1) {
        this.map.setView(this.markers[0].getLatLng(), 13);
        this.markers[0].addTo(this.map);
      } else {
        const bounds = L.latLngBounds(this.markers.map(marker => marker.getLatLng()));
        this.map.fitBounds(bounds);
        this.markers.forEach(marker => marker.addTo(this.map));
      }
    }
  }
}