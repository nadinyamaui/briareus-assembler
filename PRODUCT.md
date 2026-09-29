# Briareus for Windows
<!-- impeccable:product-schema 1 -->

## Platform
Native Windows x86-64, explicitly selected by the user.

## Stack
Assembly language, explicitly requested. FASM, Win32, WinHTTP and Windows Credential Manager.

## Product Purpose
Replicate the Briareus iOS client on Windows: connect to the existing mobile API and work with projects, conversations and the project board.

## Evidence on Hand
Reference cloned from https://github.com/okanetsolutions/briareus-ios at 0c910c71450804ef19ed651dcaf71d9ed5cf60c2 into reference/briareus-ios.
The reference UI and API contract are the authority. No real server credentials have been supplied.

## Capabilities and Constraints
HTTPS only, per-device tokens, read-only/manage access, server-advertised capabilities, no automatic mutation retries. User data must not be embedded in source or tests.

## Brand Commitments
Preserve Briareus naming and the source application's warm neutral and clay visual identity, adapted to native desktop controls.
