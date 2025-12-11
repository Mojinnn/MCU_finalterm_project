
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';
import 'dart:async';
import 'dart:math' as math;

void main() {
  runApp(const PomodoroApp());
}

class PomodoroApp extends StatelessWidget {
  const PomodoroApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ESP32 Pomodoro Timer',
      theme: ThemeData(
        primarySwatch: Colors.red,
        brightness: Brightness.dark,
        scaffoldBackgroundColor: const Color(0xFF0a0e27),
        fontFamily: 'Roboto',
      ),
      home: const PomodoroHomePage(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class PomodoroHomePage extends StatefulWidget {
  const PomodoroHomePage({Key? key}) : super(key: key);

  @override
  State<PomodoroHomePage> createState() => _PomodoroHomePageState();
}

class _PomodoroHomePageState extends State<PomodoroHomePage> with TickerProviderStateMixin {
  // Địa chỉ IP của ESP32 - THAY ĐỔI IP NÀY
  String esp32Ip = '192.168.1.100';
  
  // Dữ liệu từ ESP32
  String currentTime = '--:--:--';
  String currentDate = '--/--/----';
  String pomodoroTimer = '25:00';
  String pomodoroState = 'Ready to Work';
  String pomodoroEmoji = '💪';
  bool isRunning = false;
  int pomodoroCount = 0;
  bool isConnected = false;
  
  Timer? _updateTimer;
  late AnimationController _pulseController;
  late AnimationController _rotationController;
  late Animation<double> _pulseAnimation;
  late Animation<double> _rotationAnimation;
  
  @override
  void initState() {
    super.initState();
    
    // Animation cho pulse effect
    _pulseController = AnimationController(
      duration: const Duration(seconds: 2),
      vsync: this,
    )..repeat(reverse: true);
    
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.1).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut),
    );
    
    // Animation cho rotation
    _rotationController = AnimationController(
      duration: const Duration(seconds: 20),
      vsync: this,
    )..repeat();
    
    _rotationAnimation = Tween<double>(begin: 0, end: 2 * math.pi).animate(_rotationController);
    
    // Tự động cập nhật mỗi giây
    _updateTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      fetchData();
    });
    // Lấy dữ liệu ngay lập tức
    fetchData();
  }
  
  @override
  void dispose() {
    _updateTimer?.cancel();
    _pulseController.dispose();
    _rotationController.dispose();
    super.dispose();
  }
  
  // Lấy dữ liệu từ ESP32
  Future<void> fetchData() async {
    try {
      final response = await http.get(
        Uri.parse('http://$esp32Ip/data'),
      ).timeout(const Duration(seconds: 3));
      
      if (response.statusCode == 200) {
        final data = json.decode(response.body);
        setState(() {
          currentTime = data['time'] ?? '--:--:--';
          currentDate = data['date'] ?? '--/--/----';
          pomodoroTimer = data['timer'] ?? '25:00';
          
          // Parse state và emoji
          String fullState = data['state'] ?? 'Ready to Work';
          if (fullState.contains('💪')) {
            pomodoroEmoji = '💪';
            pomodoroState = 'Work Time';
          } else if (fullState.contains('☕')) {
            pomodoroEmoji = '☕';
            pomodoroState = 'Short Break';
          } else if (fullState.contains('🎉')) {
            pomodoroEmoji = '🎉';
            pomodoroState = 'Long Break';
          } else {
            pomodoroEmoji = '💪';
            pomodoroState = 'Ready to Work';
          }
          
          isRunning = data['running'] ?? false;
          pomodoroCount = data['count'] ?? 0;
          isConnected = true;
        });
      }
    } catch (e) {
      setState(() {
        isConnected = false;
      });
    }
  }
  
  // Start/Stop Pomodoro
  Future<void> startStop() async {
    try {
      await http.get(Uri.parse('http://$esp32Ip/start'));
      await Future.delayed(const Duration(milliseconds: 200));
      fetchData();
    } catch (e) {
      showError('Không thể kết nối với ESP32');
    }
  }
  
  // Reset Pomodoro
  Future<void> reset() async {
    try {
      await http.get(Uri.parse('http://$esp32Ip/reset'));
      await Future.delayed(const Duration(milliseconds: 200));
      fetchData();
    } catch (e) {
      showError('Không thể kết nối với ESP32');
    }
  }
  

// Thay thế hàm setTime hiện tại bằng phiên bản này:

// Set Time - Phiên bản cải tiến
Future<void> setTime(DateTime dateTime) async {
  try {
    print('Đang gửi request set time...');
    
    final body = json.encode({
      'h': dateTime.hour,
      'm': dateTime.minute,
      's': dateTime.second,
      'd': dateTime.day,
      'mo': dateTime.month,
      'y': dateTime.year,
    });
    
    print('Body: $body');
    
    final response = await http.post(
      Uri.parse('http://$esp32Ip/settime'),
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
      },
      body: body,
    ).timeout(
      const Duration(seconds: 5),
      onTimeout: () {
        throw TimeoutException('Request timeout');
      },
    );
    
    print('Response status: ${response.statusCode}');
    print('Response body: ${response.body}');
    
    if (response.statusCode == 200) {
      await Future.delayed(const Duration(milliseconds: 500));
      await fetchData();
      
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('✓ Đã cập nhật thời gian thành công'),
            backgroundColor: Colors.green,
            duration: Duration(seconds: 2),
          ),
        );
      }
    } else {
      throw Exception('Server returned status: ${response.statusCode}');
    }
  } on TimeoutException catch (e) {
    print('Timeout error: $e');
    showError('Timeout: ESP32 không phản hồi');
  } on http.ClientException catch (e) {
    print('Client error: $e');
    showError('Lỗi kết nối: ${e.message}');
  } catch (e) {
    print('Error setting time: $e');
    showError('Không thể cập nhật thời gian: $e');
  }
}
  
  void showError(String message) {
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(message),
          backgroundColor: Colors.red,
          duration: const Duration(seconds: 2),
        ),
      );
    }
  }
  
  // Dialog cài đặt IP
  void showIpDialog() {
    final controller = TextEditingController(text: esp32Ip);
    
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF16213e),
        title: const Text('Cài đặt địa chỉ IP ESP32'),
        content: TextField(
          controller: controller,
          decoration: const InputDecoration(
            hintText: '192.168.1.100',
            labelText: 'Địa chỉ IP',
            border: OutlineInputBorder(),
          ),
          keyboardType: TextInputType.numberWithOptions(decimal: true),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Hủy'),
          ),
          ElevatedButton(
            onPressed: () {
              setState(() {
                esp32Ip = controller.text;
              });
              Navigator.pop(context);
              fetchData();
            },
            child: const Text('Lưu'),
          ),
        ],
      ),
    );
  }
  
  // Dialog cài đặt thời gian
  void showSetTimeDialog() async {
    DateTime selectedDate = DateTime.now();
    TimeOfDay selectedTime = TimeOfDay.now();
    
    await showDialog(
      context: context,
      builder: (context) => StatefulBuilder(
        builder: (context, setDialogState) => AlertDialog(
          backgroundColor: const Color(0xFF16213e),
          title: const Text('⚙️ Cài đặt thời gian'),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              ListTile(
                title: const Text('Thời gian'),
                subtitle: Text(
                  '${selectedTime.hour.toString().padLeft(2, '0')}:${selectedTime.minute.toString().padLeft(2, '0')}',
                  style: const TextStyle(fontSize: 24, color: Colors.white),
                ),
                trailing: const Icon(Icons.access_time),
                onTap: () async {
                  final time = await showTimePicker(
                    context: context,
                    initialTime: selectedTime,
                  );
                  if (time != null) {
                    setDialogState(() {
                      selectedTime = time;
                    });
                  }
                },
              ),
              const SizedBox(height: 10),
              ListTile(
                title: const Text('Ngày'),
                subtitle: Text(
                  '${selectedDate.day.toString().padLeft(2, '0')}/${selectedDate.month.toString().padLeft(2, '0')}/${selectedDate.year}',
                  style: const TextStyle(fontSize: 20, color: Colors.white),
                ),
                trailing: const Icon(Icons.calendar_today),
                onTap: () async {
                  final date = await showDatePicker(
                    context: context,
                    initialDate: selectedDate,
                    firstDate: DateTime(2000),
                    lastDate: DateTime(2100),
                  );
                  if (date != null) {
                    setDialogState(() {
                      selectedDate = date;
                    });
                  }
                },
              ),
            ],
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text('Hủy'),
            ),
            ElevatedButton(
              onPressed: () {
                final dateTime = DateTime(
                  selectedDate.year,
                  selectedDate.month,
                  selectedDate.day,
                  selectedTime.hour,
                  selectedTime.minute,
                  0,
                );
                setTime(dateTime);
                Navigator.pop(context);
              },
              child: const Text('💾 Lưu'),
            ),
          ],
        ),
      ),
    );
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
            colors: [
              const Color(0xFF0a0e27),
              const Color(0xFF1a1a2e),
              const Color(0xFF16213e),
            ],
          ),
        ),
        child: SafeArea(
          child: RefreshIndicator(
            onRefresh: fetchData,
            color: const Color(0xFFe94560),
            child: SingleChildScrollView(
              physics: const AlwaysScrollableScrollPhysics(),
              child: Padding(
                padding: const EdgeInsets.all(20.0),
                child: Column(
                  children: [
                    // Header với animation
                    _buildAnimatedHeader(),
                    
                    const SizedBox(height: 30),
                    
                    // Connection Status
                    _buildConnectionStatus(),
                    
                    const SizedBox(height: 30),
                    
                    // Clock với gradient border
                    _buildClockCard(),
                    
                    const SizedBox(height: 30),
                    
                    // Pomodoro Timer với circular progress
                    _buildPomodoroCard(),
                    
                    const SizedBox(height: 30),
                    
                    // Control Buttons
                    _buildControlButtons(),
                    
                    const SizedBox(height: 20),
                    
                    // IP Info
                    _buildIpInfo(),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
  
  Widget _buildAnimatedHeader() {
    return AnimatedBuilder(
      animation: _rotationAnimation,
      builder: (context, child) {
        return Transform.rotate(
          angle: _rotationAnimation.value,
          child: Container(
            width: 100,
            height: 100,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              gradient: LinearGradient(
                colors: [
                  const Color(0xFFe94560),
                  const Color(0xFFf39c12),
                  const Color(0xFF27ae60),
                ],
              ),
              boxShadow: [
                BoxShadow(
                  color: const Color(0xFFe94560).withOpacity(0.5),
                  blurRadius: 30,
                  spreadRadius: 5,
                ),
              ],
            ),
            child: Center(
              child: Container(
                width: 90,
                height: 90,
                decoration: const BoxDecoration(
                  shape: BoxShape.circle,
                  color: Color(0xFF0a0e27),
                ),
                child: const Icon(
                  Icons.timer,
                  size: 50,
                  color: Colors.white,
                ),
              ),
            ),
          ),
        );
      },
    );
  }
  
  Widget _buildConnectionStatus() {
    return AnimatedContainer(
      duration: const Duration(milliseconds: 500),
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
      decoration: BoxDecoration(
        gradient: LinearGradient(
          colors: isConnected
              ? [const Color(0xFF27ae60), const Color(0xFF2ecc71)]
              : [const Color(0xFFe74c3c), const Color(0xFFc0392b)],
        ),
        borderRadius: BorderRadius.circular(30),
        boxShadow: [
          BoxShadow(
            color: (isConnected ? const Color(0xFF27ae60) : const Color(0xFFe74c3c))
                .withOpacity(0.5),
            blurRadius: 20,
            spreadRadius: 2,
          ),
        ],
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 12,
            height: 12,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: Colors.white,
              boxShadow: [
                BoxShadow(
                  color: Colors.white.withOpacity(0.8),
                  blurRadius: 8,
                  spreadRadius: 2,
                ),
              ],
            ),
          ),
          const SizedBox(width: 12),
          Text(
            isConnected ? 'Đã kết nối' : 'Mất kết nối',
            style: const TextStyle(
              color: Colors.white,
              fontWeight: FontWeight.bold,
              fontSize: 16,
              letterSpacing: 1,
            ),
          ),
          const SizedBox(width: 8),
          IconButton(
            icon: const Icon(Icons.settings, color: Colors.white, size: 20),
            onPressed: showIpDialog,
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(),
          ),
        ],
      ),
    );
  }
  
  Widget _buildClockCard() {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(30),
      decoration: BoxDecoration(
        gradient: const LinearGradient(
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
          colors: [
            Color(0xFF1e3c72),
            Color(0xFF2a5298),
          ],
        ),
        borderRadius: BorderRadius.circular(30),
        border: Border.all(
          width: 2,
          color: Colors.white.withOpacity(0.1),
        ),
        boxShadow: [
          BoxShadow(
            color: const Color(0xFF2a5298).withOpacity(0.5),
            blurRadius: 30,
            offset: const Offset(0, 10),
          ),
        ],
      ),
      child: Column(
        children: [
          // Icon
          Container(
            padding: const EdgeInsets.all(15),
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: Colors.white.withOpacity(0.1),
            ),
            child: const Icon(
              Icons.access_time,
              size: 40,
              color: Colors.white70,
            ),
          ),
          const SizedBox(height: 20),
          // Time
          ScaleTransition(
            scale: _pulseAnimation,
            child: Text(
              currentTime,
              style: const TextStyle(
                fontSize: 64,
                fontWeight: FontWeight.bold,
                color: Colors.white,
                letterSpacing: 5,
                fontFamily: 'Courier',
                shadows: [
                  Shadow(
                    color: Color(0xFF2a5298),
                    blurRadius: 30,
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: 15),
          // Date
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 8),
            decoration: BoxDecoration(
              color: Colors.white.withOpacity(0.1),
              borderRadius: BorderRadius.circular(20),
            ),
            child: Text(
              currentDate,
              style: const TextStyle(
                fontSize: 22,
                color: Colors.white70,
                letterSpacing: 2,
              ),
            ),
          ),
        ],
      ),
    );
  }
  
  Widget _buildPomodoroCard() {
    // Parse timer để tính phần trăm
    List<String> timerParts = pomodoroTimer.split(':');
    int minutes = int.tryParse(timerParts[0]) ?? 0;
    int seconds = int.tryParse(timerParts[1]) ?? 0;
    int totalSeconds = minutes * 60 + seconds;
    
    int maxSeconds = 25 * 60; // Default work duration
    if (pomodoroState.contains('Short Break')) {
      maxSeconds = 5 * 60;
    } else if (pomodoroState.contains('Long Break')) {
      maxSeconds = 15 * 60;
    }
    
    double progress = totalSeconds / maxSeconds;
    
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(30),
      decoration: BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
          colors: [
            const Color(0xFF4a148c),
            const Color(0xFF6a1b9a),
            const Color(0xFF8e24aa),
          ],
        ),
        borderRadius: BorderRadius.circular(30),
        border: Border.all(
          width: 2,
          color: Colors.white.withOpacity(0.1),
        ),
        boxShadow: [
          BoxShadow(
            color: const Color(0xFF6a1b9a).withOpacity(0.5),
            blurRadius: 30,
            offset: const Offset(0, 10),
          ),
        ],
      ),
      child: Column(
        children: [
          // State
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                pomodoroEmoji,
                style: const TextStyle(fontSize: 40),
              ),
              const SizedBox(width: 15),
              Text(
                pomodoroState,
                style: const TextStyle(
                  fontSize: 24,
                  color: Colors.white,
                  fontWeight: FontWeight.bold,
                  letterSpacing: 2,
                ),
              ),
            ],
          ),
          const SizedBox(height: 30),
          
          // Circular Progress
          Stack(
            alignment: Alignment.center,
            children: [
              SizedBox(
                width: 200,
                height: 200,
                child: CircularProgressIndicator(
                  value: progress,
                  strokeWidth: 12,
                  backgroundColor: Colors.white.withOpacity(0.1),
                  valueColor: AlwaysStoppedAnimation<Color>(
                    isRunning ? const Color(0xFF2ecc71) : const Color(0xFFe94560),
                  ),
                ),
              ),
              Column(
                children: [
                  Text(
                    pomodoroTimer,
                    style: const TextStyle(
                      fontSize: 56,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                      fontFamily: 'Courier',
                      letterSpacing: 3,
                      shadows: [
                        Shadow(
                          color: Colors.black45,
                          blurRadius: 10,
                        ),
                      ],
                    ),
                  ),
                  const SizedBox(height: 10),
                  Container(
                    padding: const EdgeInsets.symmetric(
                      horizontal: 16,
                      vertical: 6,
                    ),
                    decoration: BoxDecoration(
                      color: isRunning
                          ? const Color(0xFF2ecc71)
                          : const Color(0xFFe94560),
                      borderRadius: BorderRadius.circular(20),
                      boxShadow: [
                        BoxShadow(
                          color: (isRunning
                                  ? const Color(0xFF2ecc71)
                                  : const Color(0xFFe94560))
                              .withOpacity(0.6),
                          blurRadius: 15,
                        ),
                      ],
                    ),
                    child: Text(
                      isRunning ? 'ĐANG CHẠY' : 'TẠM DỪNG',
                      style: const TextStyle(
                        color: Colors.white,
                        fontWeight: FontWeight.bold,
                        fontSize: 14,
                        letterSpacing: 1.5,
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
          
          const SizedBox(height: 30),
          
          // Pomodoro Count
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            decoration: BoxDecoration(
              color: Colors.white.withOpacity(0.1),
              borderRadius: BorderRadius.circular(25),
              border: Border.all(
                color: Colors.white.withOpacity(0.2),
                width: 1,
              ),
            ),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                const Text(
                  '🏅',
                  style: TextStyle(fontSize: 24),
                ),
                const SizedBox(width: 12),
                Text(
                  'Pomodoros: $pomodoroCount',
                  style: const TextStyle(
                    fontSize: 20,
                    color: Colors.white,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
  
  Widget _buildControlButtons() {
    return Column(
      children: [
        Row(
          children: [
            Expanded(
              child: _buildGradientButton(
                icon: Icons.play_arrow,
                label: 'Start/Stop',
                gradient: const LinearGradient(
                  colors: [Color(0xFF27ae60), Color(0xFF2ecc71)],
                ),
                onPressed: isConnected ? startStop : null,
              ),
            ),
            const SizedBox(width: 15),
            Expanded(
              child: _buildGradientButton(
                icon: Icons.refresh,
                label: 'Reset',
                gradient: const LinearGradient(
                  colors: [Color(0xFFf39c12), Color(0xFFe67e22)],
                ),
                onPressed: isConnected ? reset : null,
              ),
            ),
          ],
        ),
        const SizedBox(height: 15),
        SizedBox(
          width: double.infinity,
          child: _buildGradientButton(
            icon: Icons.schedule,
            label: 'Cài đặt thời gian',
            gradient: const LinearGradient(
              colors: [Color(0xFF3498db), Color(0xFF2980b9)],
            ),
            onPressed: isConnected ? showSetTimeDialog : null,
          ),
        ),
      ],
    );
  }
  
  Widget _buildGradientButton({
    required IconData icon,
    required String label,
    required Gradient gradient,
    required VoidCallback? onPressed,
  }) {
    return Container(
      decoration: BoxDecoration(
        gradient: onPressed != null ? gradient : null,
        color: onPressed == null ? Colors.grey.withOpacity(0.3) : null,
        borderRadius: BorderRadius.circular(20),
        boxShadow: onPressed != null
            ? [
                BoxShadow(
                  color: Colors.black.withOpacity(0.3),
                  blurRadius: 15,
                  offset: const Offset(0, 5),
                ),
              ]
            : [],
      ),
      child: Material(
        color: Colors.transparent,
        child: InkWell(
          onTap: onPressed,
          borderRadius: BorderRadius.circular(20),
          child: Padding(
            padding: const EdgeInsets.symmetric(vertical: 18),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(icon, color: Colors.white, size: 24),
                const SizedBox(width: 10),
                Text(
                  label,
                  style: const TextStyle(
                    color: Colors.white,
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                    letterSpacing: 1,
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
  
  Widget _buildIpInfo() {
    return Container(
      padding: const EdgeInsets.all(15),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.05),
        borderRadius: BorderRadius.circular(15),
        border: Border.all(
          color: Colors.white.withOpacity(0.1),
        ),
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Icon(
            Icons.wifi,
            color: Colors.white38,
            size: 16,
          ),
          const SizedBox(width: 8),
          Text(
            'ESP32: $esp32Ip',
            style: const TextStyle(
              color: Colors.white38,
              fontSize: 12,
              fontFamily: 'Courier',
            ),
          ),
        ],
      ),
    );
  }
}

