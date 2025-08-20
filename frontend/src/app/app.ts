import { Component, signal } from '@angular/core';
import { RouterOutlet, RouterLink } from '@angular/router';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, CommonModule, RouterLink],
  templateUrl: './app.html',
  styleUrl: './app.css'
})
export class App {
  readonly title = signal('frontend');

  // Controls mobile sidebar visibility (and acts as expand toggle on small screens)
  readonly isSidebarOpen = signal(false);

  // Accordion state for GPS group
  readonly isGpsOpen = signal(true);

  constructor() {}

  toggleSidebar(): void {
    this.isSidebarOpen.update((current) => !current);
  }

  closeSidebar(): void {
    this.isSidebarOpen.set(false);
  }

  toggleGps(): void {
    this.isGpsOpen.update((current) => !current);
  }

  // Accordion header now only toggles open/close via toggleGps()
}
