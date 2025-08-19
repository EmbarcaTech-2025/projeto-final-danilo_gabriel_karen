import { Component, signal } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, CommonModule],
  templateUrl: './app.html',
  styleUrl: './app.css'
})
export class App {
  readonly title = signal('frontend');

  // Controls mobile sidebar visibility (and acts as expand toggle on small screens)
  readonly isSidebarOpen = signal(false);

  toggleSidebar(): void {
    this.isSidebarOpen.update((current) => !current);
  }

  closeSidebar(): void {
    this.isSidebarOpen.set(false);
  }
}
