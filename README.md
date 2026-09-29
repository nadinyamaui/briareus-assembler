# Briareus para Windows x86-64, en assembler

Cliente nativo escrito en ensamblador x86-64 con FASM. Genera un ejecutable PE64 y utiliza Win32, WinHTTP y Windows Credential Manager. No ejecuta Swift, C, Python, JavaScript, Electron ni una web embebida.

**Estado: versión inicial funcional; todavía no tiene paridad completa con la aplicación iOS.** Los flujos principales tienen interfaz nativa. Las operaciones avanzadas se ejecutan desde un editor JSON con plantillas. Consulta la [matriz de paridad](docs/PARITY.md) antes de utilizarla como sustituto de la app original.

## Ejecutar

Abre `build/Briareus.exe` en Windows 10/11 de 64 bits. No necesita instalación ni permisos de administrador.

1. Introduce la dirección HTTPS de tu servidor Briareus.
2. Introduce un token de dispositivo creado en **Settings → Mobile devices** del dashboard.
3. Pulsa **Connect** y selecciona un proyecto.
4. Utiliza **Conversations**, **Pull requests** o **Issues**. Escribe en **Message** para enviar un mensaje o iniciar una conversación con la configuración predeterminada del proyecto.
5. **Actions / API** ofrece el catálogo de operaciones autorizado para ese dispositivo. Selecciona una operación, revisa y completa sus argumentos JSON y pulsa **Execute**. Las escrituras requieren confirmación en ese panel.

`Disconnect` elimina la conexión local; no cancela agentes ni revoca el token del servidor. `Revoke device token` revoca el token remotamente y desconecta al confirmarse la respuesta.

## Compilar y comprobar

```powershell
.\build.ps1
.\build.ps1 -Run
```

El script descarga FASM 1.73.32 desde su distribución oficial solo si falta, verifica su SHA-256, ensambla la app y ejecuta las pruebas de JSON y validación de conexión. Todo queda en este directorio. El ejecutable no depende de FASM.

Pruebas de integración opcionales: Python 3 y Pillow para las capturas.

```powershell
python -m pip install Pillow
.\test.ps1
```

Cierra otras instancias de Briareus antes de las pruebas de interfaz. La compilación `Briareus-test.exe` utiliza respuestas sintéticas, no accede a la red ni lee/escribe credenciales del usuario. `Briareus.exe` utiliza exclusivamente el transporte HTTPS real: no tiene un modo demo que evite TLS.

`scripts-generate.py` regenera las declaraciones repetitivas de controles, ya incluidas en `src/`. Python solo participa en generación y pruebas, nunca en la aplicación.

## Código

| Archivo | Responsabilidad |
| --- | --- |
| `src/main.asm` | Entrada PE64, datos, importaciones Win32, manifiesto |
| `src/ui.inc` | Ventana, navegación, estado, presentación y acciones |
| `src/net.inc` | HTTPS en un hilo de trabajo, validación, credenciales |
| `src/json.inc` | Analizador JSON UTF-16, búsqueda y construcción escapada |
| `src/templates.inc` y `src/templates-data.inc` | Argumentos editables para operaciones de la API |
| `tests/` | Pruebas assembler y automatización Win32 de la propia app |

La ventana conserva la identidad cálida del original, con controles de Windows y una distribución de tres columnas. La interfaz mantiene el inglés de la app de referencia.

## Referencia y contrato

Se clonó [okanetsolutions/briareus-ios](https://github.com/okanetsolutions/briareus-ios) en `reference/briareus-ios`, revisión `0c910c71450804ef19ed651dcaf71d9ed5cf60c2`. Ese clon permanece independiente y está excluido del repositorio principal.

```powershell
git clone https://github.com/okanetsolutions/briareus-ios.git reference/briareus-ios
```

El contrato proviene de `Core/APIClient.swift`, `Core/Models.swift`, `Core/Board.swift` y las llamadas de las pantallas SwiftUI. No se modificó el servidor ni se realizaron acciones en cuentas reales.

Fuentes de implementación: [distribución de FASM](https://flatassembler.net/download.php), [opciones de WinHTTP](https://learn.microsoft.com/en-us/windows/win32/winhttp/option-flags), [autenticación de WinHTTP](https://learn.microsoft.com/en-us/windows/win32/winhttp/authentication-in-winhttp), [mensajes de controles de edición](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-gettext).
