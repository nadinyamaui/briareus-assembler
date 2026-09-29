# Cobertura y diferencias conocidas

Referencia: Briareus iOS, commit `0c910c71450804ef19ed651dcaf71d9ed5cf60c2`.

Esta entrega no debe describirse como una rÃ©plica completa ni como validada contra un servidor Briareus real. Las pruebas de flujo usan respuestas sintÃ©ticas con el contrato del cliente original.

| FunciÃ³n del original | Windows assembler |
| --- | --- |
| HTTPS, validaciÃ³n de token, descubrimiento v1 | Implementado |
| CatÃ¡logo de operaciones y permisos read/manage | Implementado; las escrituras no se ofrecen a read-only |
| Token persistente | Windows Credential Manager, despuÃ©s de validar conexiÃ³n y catÃ¡logo |
| Olvidar / revocar conexiÃ³n | Implementado, con confirmaciÃ³n |
| Proyectos, conversaciones, bÃºsqueda | Listas nativas; bÃºsqueda sobre los datos de cada fila |
| TranscripciÃ³n de conversaciÃ³n, herramientas y preguntas | Texto Unicode con fecha; se omiten eventos setup/status |
| ActualizaciÃ³n en primer plano | Cada 7 s para la conversaciÃ³n seleccionada; se detiene al perder foco y tras errores |
| Enviar mensaje / iniciar conversaciÃ³n | Controles nativos; configuraciÃ³n predeterminada del proyecto |
| Proveedor, modelo, esfuerzo y rama | Operaciones `runtimes` / `branches` y argumentos JSON editables de `start_session` |
| Rename, cancel, close, reopen, delete | Panel Actions / API, con plantillas y confirmaciÃ³n |
| Review loop y triage | Operaciones y argumentos JSON; sin tarjetas dedicadas |
| PRs e issues | Listas; descripciÃ³n de PR/issue y datos estructurados |
| Checks, reviews, commits, etiquetas y stacks | Datos que devuelve la API, mediante presentaciÃ³n JSON; sin componentes grÃ¡ficos especÃ­ficos |
| Diffs | `pull_files` muestra cada patch en texto monoespaciado |
| PaginaciÃ³n de archivos | Manual mediante `page`, `headSha` y `baseSha` en Actions / API |
| Merge y tareas del board | Operaciones del catÃ¡logo; plantillas editables, confirmaciÃ³n y controles del servidor |
| SelecciÃ³n de issue para nueva sesiÃ³n | El usuario redacta el prompt o configura `start_session`; no hay botÃ³n que construya automÃ¡ticamente el prompt del issue |
| Markdown rico, resaltado de cÃ³digo, enlaces navegables | No implementado: texto y JSON |
| Filtrar por autor/reviewer/label | BÃºsqueda textual sobre la fila JSON; sin filtros facetados ni conteos |
| CachÃ© persistente y transcript incremental | Pendiente: se solicita de nuevo la conversaciÃ³n completa; no se guardan transcripts en disco |
| Notas de voz / transcripciÃ³n de audio | Pendiente |
| Temas claro/oscuro, escalado por monitor, modo compacto | Tema claro; DPI de sistema; ventana mÃ­nima 1100 Ã— 760 |
| Cubierta de privacidad al cambiar de app | Pendiente |
| DistribuciÃ³n firmada / instalador | No se ha firmado ni creado un instalador; ejecutable portable |

## Seguridad y lÃ­mites

- WinHTTP valida TLS con Windows; no hay opciÃ³n para ignorar errores de certificado.
- Se deshabilitan cookies, redirecciones y autenticaciÃ³n automÃ¡tica. El token se envÃ­a como Bearer solo al origen HTTPS configurado.
- No existen reintentos de escrituras en el cÃ³digo de la aplicaciÃ³n. Tras un error de escritura se bloquean los botones de envÃ­o hasta un refresco explÃ­cito; debe verificarse el resultado en el servidor antes de repetirla.
- La interfaz permanece disponible mientras un hilo efectÃºa la peticiÃ³n. No hay cancelaciÃ³n individual del transporte; cerrar la aplicaciÃ³n termina el proceso.
- LÃ­mite de peticiÃ³n UTF-8: 1 MiB. Respuesta: menos de 4 MiB. JSON: 32 767 tokens y profundidad 64. Listas: 256 proyectos, 512 filas, 255 operaciones. La vista de detalle estÃ¡ limitada a 262 143 caracteres UTF-16; campos individuales a 32 767 y snapshots de filas a 16 383.
- Los lÃ­mites no equivalen a paginaciÃ³n automÃ¡tica. Repositorios o transcripts grandes pueden requerir el dashboard web.
- Una conexiÃ³n por usuario Windows. El secreto usa Credential Manager con persistencia local; los textos de conversaciones permanecen en memoria mientras la app estÃ¡ abierta.
- La API del servidor debe seguir validando permisos, argumentos, estado de PR y condiciones de merge. Las plantillas avanzadas incluyen campos vacÃ­os que el usuario debe completar a partir del resultado actual; no inventan SHAs, ramas ni decisiones.

## ComprobaciÃ³n realizada

- CompilaciÃ³n nativa PE64 y ejecuciÃ³n de pruebas assembler de JSON y URL/token.
- IntegraciÃ³n del ejecutable real con transporte de fixtures enlazado Ãºnicamente en la compilaciÃ³n de pruebas: conexiÃ³n, proyectos, filtros, Unicode, envÃ­o exacto del texto, nueva conversaciÃ³n, conservaciÃ³n del borrador, PR, diffs, issues, rechazo de JSON incorrecto y bloqueo read-only.
- Capturas de ventanas nativas de conexiÃ³n, conversaciÃ³n, PR, diff y read-only. Prueba local de rechazo de direcciones HTTP y tokens malformados.
- Transporte real: un certificado expirado produjo el error TLS de Windows 12175; una respuesta HTML fue rechazada como no JSON. Se usÃ³ exclusivamente un token ficticio.
- Pendiente: conexiÃ³n y aceptaciÃ³n con servidor real, Credential Manager en un emparejamiento real, revocaciÃ³n real, operaciones pagadas o de GitHub y paridad pendiente de la tabla.

- Adaptación responsive comprobada en ocho tamaños, desde 480 × 680 hasta 1600 × 1000, incluidos ambos lados del cambio de distribución. Las pruebas verifican límites de controles, ausencia de solapamientos, navegación compacta y conservación de borradores y JSON.
