#include <WiFi.h>
#include <ESP32Servo.h>  // Библиотека для работы с сервоприводом

// Настройки точки доступа
const char* ap_ssid = "ESP32_AP";       // Название сети
const char* ap_password = "12345678";  // Пароль (минимум 8 символов)

// Номер порта для сервера
WiFiServer server(80);

// Состояние выходов
bool pumpState = false;  // Состояние насоса
bool windState = false;  // Состояние вентилятора
bool windowState = false;  // Состояние форточки

// Номера выводов
const int pumpPin = 26;  // Пин для насоса
const int windPin = 27;  // Пин для вентилятора
const int windowPin = 18;  // Пин для сервопривода (форточка)
const int rgbPin = 19;     // Пин для RGB-светодиода

// Объект для управления сервоприводом
Servo servoWindow;

// Переменная для хранения выбранного цвета RGB
String rgbColor = "#FFFFFF";  // Белый цвет по умолчанию

// Прототипы функций
void handleRequest(WiFiClient& client, const String& request);
void sendResponse(WiFiClient& client);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Настройка GPIO как выходы
    pinMode(pumpPin, OUTPUT);
    pinMode(windPin, OUTPUT);
    pinMode(rgbPin, OUTPUT);

    // Устанавливаем GPIO в LOW
    digitalWrite(pumpPin, LOW);
    digitalWrite(windPin, LOW);

    // Создание точки доступа
    if (WiFi.softAP(ap_ssid, ap_password)) {
        Serial.println("Access Point started.");
        Serial.print("Connect to SSID: ");
        Serial.println(ap_ssid);
        Serial.print("Password: ");
        Serial.println(ap_password);

        // Фиксированный IP-адрес для точки доступа
        Serial.println("IP address: 192.168.4.1");
    } else {
        Serial.println("Failed to create Access Point.");
    }

    

    // Инициализация сервопривода
    servoWindow.attach(windowPin);  // Подключение сервопривода к пину 18
    servoWindow.write(0);           // Закрываем форточку по умолчанию

    // Запуск сервера
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        String request = "";

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                request += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        handleRequest(client, request);
                        request = "";
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        client.stop();
        Serial.println("Client disconnected.");
    }
}

void handleRequest(WiFiClient& client, const String& request) {
    // Обработка команд управления GPIO
    if (request.indexOf("/pump/on") != -1) {
        digitalWrite(pumpPin, HIGH);
        pumpState = true;
        Serial.println("Насос ВКЛ");
    } else if (request.indexOf("/pump/off") != -1) {
        digitalWrite(pumpPin, LOW);
        pumpState = false;
        Serial.println("Насос ВЫКЛ");
    } else if (request.indexOf("/wind/on") != -1) {
        digitalWrite(windPin, HIGH);
        windState = true;
        Serial.println("Вентилятор ВКЛ");
    } else if (request.indexOf("/wind/off") != -1) {
        digitalWrite(windPin, LOW);
        windState = false;
        Serial.println("Вентилятор ВЫКЛ");
    } else if (request.indexOf("/window/open") != -1) {
        servoWindow.write(90);  // Открываем форточку
        windowState = true;
        Serial.println("Форточка ОТКРЫТА");
    } else if (request.indexOf("/window/close") != -1) {
        servoWindow.write(0);  // Закрываем форточку
        windowState = false;
        Serial.println("Форточка ЗАКРЫТА");
    } else if (request.indexOf("/rgb/color/") != -1) {
        // Извлекаем цвет из запроса
        int start = request.indexOf("/rgb/color/") + 11;
        int end = request.indexOf(' ', start);
        rgbColor = request.substring(start, end);
        Serial.println("Выбранный цвет: " + rgbColor);

        // Отправляем цвет на RGB-светодиод (пример реализации)
        uint32_t color = hexToRgb(rgbColor);
        analogWrite(rgbPin, (color >> 16) & 0xFF);  // Красный компонент
    }

    // Отправка HTTP-ответа
    sendResponse(client);
}

uint32_t hexToRgb(String hexColor) {
    // Преобразование шестнадцатеричного цвета в RGB
    hexColor = hexColor.substring(1);  // Убираем символ '#'
    int red = strtol(&hexColor[0], NULL, 16);
    int green = strtol(&hexColor[2], NULL, 16);
    int blue = strtol(&hexColor[4], NULL, 16);
    return (red << 16) | (green << 8) | blue;
}

void sendResponse(WiFiClient& client) {
    // Отправляем HTTP-заголовки
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    // Отправляем HTML-страницу
    client.println("<!DOCTYPE html>");
    client.println("<html lang=\"ru\">");
    client.println("<head>");
    client.println("<meta charset=\"UTF-8\">");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
    client.println("<title>Умная теплица</title>");
    client.println("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css\">");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; background-color: #2b19a4; color: #ffffff; margin: 0; padding: 0; }");
    client.println(".container { max-width: 1200px; margin: 0 auto; padding: 20px; display: flex; flex-wrap: wrap; justify-content: space-between; }");
    client.println(".block-tab { display: flex; flex-wrap: wrap; width: 100%; }");
    client.println(".block-thin-tab { background-color: rgb(29, 134, 146); border-radius: 8px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); margin: 10px; padding: 15px; width: calc(33.333% - 20px); box-sizing: auto; }");
    client.println(".sensor-value { display: flex; align-items: center; justify-content: space-between; padding: 10px 0; }");
    client.println(".sensor-value i { font-size: 24px; margin-right: 10px; }");
    client.println(".sensor-value span { font-size: 18px; }");
    client.println(".slider-container, .switch-container, .color-container { margin-top: 10px; }");
    client.println(".switch input { display: none; }");
    client.println(".switch .slider { position: relative; display: inline-block; width: 60px; height: 34px; background-color: #ccc; border-radius: 34px; transition: background-color 0.3s; cursor: pointer; }");
    client.println(".switch .slider:before { content: ''; position: absolute; left: 4px; top: 4px; width: 26px; height: 26px; background-color: white; border-radius: 50%; transition: left 0.3s; }");
    client.println(".switch input:checked + .slider { background-color: #27ae60; }");
    client.println(".switch input:checked + .slider:before { left: 30px; }");
    client.println(".color-container input[type='color'] { border: none; padding: 0; width: 50px; height: 50px; cursor: pointer; }");
    client.println(".icon-normal { color: green; }");
    client.println(".icon-low { color: blue; }");
    client.println(".icon-high { color: red; }");
    client.println(".icon-low-humidity { color: yellow; }");
    client.println(".icon-high-humidity { color: cyan; }");
    client.println(".icon-low-light { color: yellow; }");
    client.println(".icon-high-light { color: orange; }");
    client.println(".icon-moderate-uv { color: yellow; }");
    client.println(".icon-high-uv { color: red; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<div class=\"container\">");
    client.println("<h1>Умная теплица</h1>");
    client.println("<div class=\"block-tab\">");

    // Температура
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"sensor-value\" id=\"temperature-sensor\">");
    client.println("<i class=\"fas fa-temperature-low\"></i>");
    client.println("<span id=\"tGH\">0</span><span>°C</span>");
    client.println("</div>");
    client.println("</div>");

    // Влажность
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"sensor-value\" id=\"humidity-sensor\">");
    client.println("<i class=\"fas fa-tint\"></i>");
    client.println("<span id=\"humGH\">0</span><span>%</span>");
    client.println("</div>");
    client.println("</div>");

    // Освещенность
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"sensor-value\" id=\"light-sensor\">");
    client.println("<i class=\"fas fa-sun\"></i>");
    client.println("<span id=\"lightGH\">0</span><span>lx</span>");
    client.println("</div>");
    client.println("<div class=\"slider-container\">");
    client.println("<label for=\"window\">Освещенность:</label>");
    client.println("<input type=\"range\" id=\"window\" min=\"0\" max=\"100\" value=\"50\">");
    client.println("<span id=\"window-value\">50%</span>");
    client.println("</div>");
    client.println("<div class=\"color-container\">");
    client.println("<label for=\"colorGH\">Выбрать цвет:</label>");
    client.println("<input type=\"color\" id=\"colorGH\" value=\"" + rgbColor + "\">");
    client.println("</div>");
    client.println("</div>");

    // Насос
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"switch-container\">");
    client.println("<label for=\"pump\">Насос:</label>");
    client.println("<label class=\"switch\">");
    client.println(String("<input type=\"checkbox\" id=\"pump\" ") + (pumpState ? "checked" : "") + ">"); // Добавляем состояние насоса
    client.println("<span class=\"slider round\"></span>");
    client.println("</label>");
    client.println("</div>");
    client.println("</div>");

    // Вентилятор
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"switch-container\">");
    client.println("<label for=\"wind\">Вентилятор:</label>");
    client.println("<label class=\"switch\">");
    client.println(String("<input type=\"checkbox\" id=\"wind\" ") + (windState ? "checked" : "") + ">"); // Добавляем состояние вентилятора
    client.println("<span class=\"slider round\"></span>");
    client.println("</label>");
    client.println("</div>");
    client.println("</div>");

    // Форточка
    client.println("<div class=\"block-thin-tab\">");
    client.println("<div class=\"switch-container\">");
    client.println("<label for=\"window\">Форточка:</label>");
    client.println("<label class=\"switch\">");
    client.println(String("<input type=\"checkbox\" id=\"window\" ") + (windowState ? "checked" : "") + ">"); // Добавляем состояние форточки
    client.println("<span class=\"slider round\"></span>");
    client.println("</label>");
    client.println("</div>");
    client.println("</div>");

    client.println("</div>");
    client.println("</div>");

    // JavaScript для обновления значений
    client.println("<script>");
    client.println("const windowSlider = document.getElementById('window');");
    client.println("const windowValue = document.getElementById('window-value');");
    client.println("windowSlider.addEventListener('input', function() {");
    client.println("windowValue.textContent = `${this.value}%`;}");
    client.println(");");
    client.println("const pumpSwitch = document.getElementById('pump');");
    client.println("pumpSwitch.addEventListener('change', function() {");
    client.println("console.log(`Насос: ${this.checked ? 'Вкл' : 'Выкл'}`);");
    client.println("fetch(`/pump/${this.checked ? 'on' : 'off'}`);");
    client.println("});");
    client.println("const windSwitch = document.getElementById('wind');");
    client.println("windSwitch.addEventListener('change', function() {");
    client.println("console.log(`Вентилятор: ${this.checked ? 'Вкл' : 'Выкл'}`);");
    client.println("fetch(`/wind/${this.checked ? 'on' : 'off'}`);");
    client.println("});");
    client.println("const windowSwitch = document.getElementById('window');");
    client.println("windowSwitch.addEventListener('change', function() {");
    client.println("console.log(`Форточка: ${this.checked ? 'Открыта' : 'Закрыта'}`);");
    client.println("fetch(`/window/${this.checked ? 'open' : 'close'}`);");
    client.println("});");
    client.println("const colorPicker = document.getElementById('colorGH');");
    client.println("colorPicker.addEventListener('input', function() {");
    client.println("console.log(`Выбранный цвет: ${this.value}`);");
    client.println("fetch(`/rgb/color/${this.value}`);");
    client.println("});");
    client.println("function updateSensorValues() {");
    client.println("const temperature = Math.floor(Math.random() * 50 - 10);");
    client.println("const humidity = Math.floor(Math.random() * 100);");
    client.println("const light = Math.floor(Math.random() * 1000);");
    client.println("document.getElementById('tGH').textContent = temperature;");
    client.println("document.getElementById('humGH').textContent = humidity;");
    client.println("document.getElementById('lightGH').textContent = light;");
    client.println("updateIconColor('temperature-sensor', temperature, [");
    client.println("{ condition: temperature < 10, className: 'icon-low' },");
    client.println("{ condition: temperature > 30, className: 'icon-high' },");
    client.println("{ condition: true, className: 'icon-normal' }]);");
    client.println("updateIconColor('humidity-sensor', humidity, [");
    client.println("{ condition: humidity < 30, className: 'icon-low-humidity' },");
    client.println("{ condition: humidity > 70, className: 'icon-high-humidity' },");
    client.println("{ condition: true, className: 'icon-normal' }]);");
    client.println("updateIconColor('light-sensor', light, [");
    client.println("{ condition: light < 200, className: 'icon-low-light' },");
    client.println("{ condition: light > 800, className: 'icon-high-light' },");
    client.println("{ condition: true, className: 'icon-normal' }]);");
    client.println("}");
    client.println("function updateIconColor(sensorId, value, conditions) {");
    client.println("const sensorElement = document.getElementById(sensorId);");
    client.println("const iconElement = sensorElement.querySelector('i');");
    client.println("iconElement.classList.remove('icon-normal', 'icon-low', 'icon-high', 'icon-low-humidity', 'icon-high-humidity', 'icon-low-light', 'icon-high-light', 'icon-moderate-uv', 'icon-high-uv');");
    client.println("let matchedCondition = null;");
    client.println("for (const condition of conditions) {");
    client.println("if (typeof condition.condition === 'function' ? condition.condition(value) : condition.condition) {");
    client.println("matchedCondition = condition.className;");
    client.println("break;");
    client.println("}");
    client.println("}");
    client.println("if (matchedCondition) {");
    client.println("iconElement.classList.add(matchedCondition);");
    client.println("} else {");
    client.println("iconElement.classList.add('icon-normal');");
    client.println("}");
    client.println("}");
    client.println("setInterval(updateSensorValues, 1000);");
    client.println("</script>");
    client.println("</body>");
    client.println("</html>");
}