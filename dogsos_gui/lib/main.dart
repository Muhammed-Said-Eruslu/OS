import 'dart:io';
import 'dart:ui';
import 'package:flutter/material.dart';

void main() {
  runApp(const DogsOS());
}

class DogsOS extends StatelessWidget {
  const DogsOS({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'DogsOS Desktop',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        fontFamily: 'SF Pro Display',
        brightness: Brightness.dark,
        scaffoldBackgroundColor: Colors.transparent,
      ),
      home: const DogsDesktop(),
    );
  }
}

class DogsDesktop extends StatefulWidget {
  const DogsDesktop({super.key});

  @override
  State<DogsDesktop> createState() => _DogsDesktopState();
}

class _DogsDesktopState extends State<DogsDesktop> with TickerProviderStateMixin {
  bool _booting = true;
  late AnimationController _fadeController;

  @override
  void initState() {
    super.initState();
    _fadeController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1500),
    )..forward();

    Future.delayed(const Duration(seconds: 2), () {
      if (mounted) {
        setState(() => _booting = false);
      }
    });
  }

  @override
  void dispose() {
    _fadeController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Stack(
        children: [
          // 🎨 Sabit Arka Plan (flicker-free)
          Container(
            decoration: const BoxDecoration(
              gradient: LinearGradient(
                colors: [
                  Color(0xFF1a1a2e),
                  Color(0xFF16213e),
                  Color(0xFF0f3460),
                  Color(0xFF533483),
                ],
                begin: Alignment.topLeft,
                end: Alignment.bottomRight,
              ),
            ),
          ),

          // ✨ Hafif blur overlay
          Positioned.fill(
            child: BackdropFilter(
              filter: ImageFilter.blur(sigmaX: 25, sigmaY: 25),
              child: Container(color: Colors.black.withOpacity(0.15)),
            ),
          ),

          // 🚀 Boot ekranı veya 💻 Masaüstü
          _buildMainContent(),
        ],
      ),
    );
  }

  Widget _buildMainContent() {
    return AnimatedSwitcher(
      duration: const Duration(milliseconds: 1000),
      child: _booting ? _buildBootScreen() : _buildDesktop(),
    );
  }

  Widget _buildBootScreen() {
    return FadeTransition(
      key: const ValueKey('boot_screen'),
      opacity: _fadeController,
      child: Container(
        alignment: Alignment.center,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              padding: const EdgeInsets.all(30),
              decoration: BoxDecoration(
                gradient: const LinearGradient(
                  colors: [Color(0xFF667eea), Color(0xFF764ba2)],
                ),
                borderRadius: BorderRadius.circular(30),
                boxShadow: [
                  BoxShadow(
                    color: Colors.blue.withOpacity(0.4),
                    blurRadius: 40,
                  ),
                ],
              ),
              child: const Text("🐾", style: TextStyle(fontSize: 64)),
            ),
            const SizedBox(height: 24),
            const Text(
              "DogsOS",
              style: TextStyle(
                fontSize: 36,
                fontWeight: FontWeight.bold,
                letterSpacing: 1.5,
              ),
            ),
            const SizedBox(height: 8),
            const Text(
              "Starting system...",
              style: TextStyle(color: Colors.white70),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildDesktop() {
    return FadeTransition(
      key: const ValueKey('desktop'),
      opacity: _fadeController,
      child: Stack(
        fit: StackFit.expand,
        children: [
          // Üst Bar
          _buildTopBar(),
          
          // Masaüstü ikonları
          _buildDesktopIcons(),
          
          // Dock Bar
          _buildDockBar(),
        ],
      ),
    );
  }

  Widget _buildTopBar() {
    return Align(
      alignment: Alignment.topCenter,
      child: Container(
        height: 44,
        decoration: BoxDecoration(
          color: Colors.black.withOpacity(0.4),
          border: Border(
            bottom: BorderSide(
              color: Colors.white.withOpacity(0.1),
            ),
          ),
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              children: const [
                SizedBox(width: 16),
                Text("🐾 DogsOS",
                    style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.w600)),
                SizedBox(width: 24),
                _MenuButton("File"),
                _MenuButton("Edit"),
                _MenuButton("View"),
                _MenuButton("Window"),
                _MenuButton("Help"),
              ],
            ),
            const Padding(
              padding: EdgeInsets.only(right: 16),
              child: Text("🕒 12:45 PM",
                  style: TextStyle(color: Colors.white70)),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildDesktopIcons() {
    return Positioned(
      left: 50,
      top: 80,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: const [
          DesktopIcon(
            label: "Terminal",
            icon: Icons.terminal_rounded,
            color: Color(0xFF00D9FF),
          ),
          SizedBox(height: 32),
          DesktopIcon(
            label: "Files",
            icon: Icons.folder_rounded,
            color: Color(0xFFFFB800),
          ),
          SizedBox(height: 32),
          DesktopIcon(
            label: "Settings",
            icon: Icons.settings_rounded,
            color: Color(0xFF8B5CF6),
          ),
          SizedBox(height: 32),
          DesktopIcon(
            label: "Browser",
            icon: Icons.language_rounded,
            color: Color(0xFFFF6B6B),
          ),
        ],
      ),
    );
  }

  Widget _buildDockBar() {
    return Align(
      alignment: Alignment.bottomCenter,
      child: Padding(
        padding: const EdgeInsets.only(bottom: 16),
        child: Container(
          height: 70,
          width: 520,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.1),
            borderRadius: BorderRadius.circular(20),
            border: Border.all(
              color: Colors.white.withOpacity(0.2),
              width: 1,
            ),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: const [
              DockIcon(icon: Icons.home, label: "Home", color: Colors.cyan),
              DockIcon(icon: Icons.folder, label: "Files", color: Colors.amber),
              DockIcon(icon: Icons.music_note, label: "Music", color: Colors.greenAccent),
              DockIcon(icon: Icons.terminal, label: "Terminal", color: Colors.blueAccent),
              DockIcon(icon: Icons.settings, label: "Settings", color: Colors.purpleAccent),
            ],
          ),
        ),
      ),
    );
  }
}

// =========================
// 🧩 COMPONENTS
// =========================

class _MenuButton extends StatefulWidget {
  final String title;
  const _MenuButton(this.title);

  @override
  State<_MenuButton> createState() => _MenuButtonState();
}

class _MenuButtonState extends State<_MenuButton> {
  bool hover = false;
  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 6),
      child: MouseRegion(
        cursor: SystemMouseCursors.click,
        onEnter: (_) => setState(() => hover = true),
        onExit: (_) => setState(() => hover = false),
        child: Text(
          widget.title,
          style: TextStyle(
            color: hover ? Colors.white : Colors.white70,
            fontWeight: hover ? FontWeight.bold : FontWeight.w500,
          ),
        ),
      ),
    );
  }
}

class DesktopIcon extends StatefulWidget {
  final String label;
  final IconData icon;
  final Color color;
  const DesktopIcon(
      {super.key, required this.label, required this.icon, required this.color});

  @override
  State<DesktopIcon> createState() => _DesktopIconState();
}

class _DesktopIconState extends State<DesktopIcon> {
  bool hover = false;

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      cursor: SystemMouseCursors.click,
      onEnter: (_) => setState(() => hover = true),
      onExit: (_) => setState(() => hover = false),
      child: GestureDetector(
        onTap: () async {
          if (widget.label == "Terminal") {
await Process.start(
  'bash',
  ['-c', 'cd /home/saide/myOS && make run'],
  mode: ProcessStartMode.detached,
);

          }
        },
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          transform: Matrix4.identity()
            ..scale(hover ? 1.05 : 1.0)
            ..translate(0.0, hover ? -5.0 : 0.0),
          child: Column(
            children: [
              Container(
                padding: const EdgeInsets.all(14),
                decoration: BoxDecoration(
                  color: widget.color.withOpacity(hover ? 0.4 : 0.25),
                  borderRadius: BorderRadius.circular(16),
                  border: Border.all(color: widget.color.withOpacity(0.5)),
                ),
                child: Icon(widget.icon, color: Colors.white, size: 38),
              ),
              const SizedBox(height: 8),
              Text(widget.label,
                  style: const TextStyle(color: Colors.white, fontSize: 13)),
            ],
          ),
        ),
      ),
    );
  }
}

class DockIcon extends StatefulWidget {
  final IconData icon;
  final String label;
  final Color color;
  const DockIcon(
      {super.key, required this.icon, required this.label, required this.color});

  @override
  State<DockIcon> createState() => _DockIconState();
}

class _DockIconState extends State<DockIcon> {
  bool hover = false;
  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      cursor: SystemMouseCursors.click,
      onEnter: (_) => setState(() => hover = true),
      onExit: (_) => setState(() => hover = false),
      child: GestureDetector(
        onTap: () async {
          if (widget.label == "Terminal") {
            await Process.start(
              'qemu-system-i386',
              ['-cdrom', '/home/saide/myOS/myos.iso', '-boot', 'd'],
              mode: ProcessStartMode.detached,
            );
          }
        },
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          transform: Matrix4.identity()
            ..scale(hover ? 1.2 : 1.0)
            ..translate(0.0, hover ? -8.0 : 0.0),
          child: Container(
            padding: const EdgeInsets.all(10),
            decoration: BoxDecoration(
              color: widget.color.withOpacity(hover ? 0.3 : 0.15),
              borderRadius: BorderRadius.circular(14),
              boxShadow: hover
                  ? [
                      BoxShadow(
                        color: widget.color.withOpacity(0.4),
                        blurRadius: 15,
                        spreadRadius: 2,
                      ),
                    ]
                  : [],
            ),
            child: Icon(widget.icon, color: Colors.white, size: 30),
          ),
        ),
      ),
    );
  }
}