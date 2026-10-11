# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 1.0.7 (current) | Yes |
| Older releases | Not published from this repository |

## Reporting a Vulnerability

Please **do not** open a public issue for security-sensitive reports when a private channel is available.

**Maintainer contact (email):** `cloudgen.wong@gmail.com`

- Source of contact: product **author-email** in [`LICENSE.md`](LICENSE.md) (Copyright line).
- Prefer email for vulnerability details, reproduction steps, and impact.
- You can also open a private advisory at <https://github.com/cloudgen/cloud-player/security/advisories/new>.
- You should receive an acknowledgment when the report is received and actionable.
- Do not include exploit weaponization guides in public channels.

## Security Design Principles (CIAO)

This project follows **[CIAO](https://github.com/cloudgen/ciao)** / **CIAO-Lite** defensive design. Security-relevant intent:

| Letter | Principle | Security application |
|--------|-----------|----------------------|
| **C** | **Caution** | Treat page content, addresses, and local paths as untrusted. Keep cookies and logs inside the per-user app folder. Fail a video load with a visible error instead of pretending it played. |
| **I** | **Intentional** | The window opens Gemini. An address ending in `.mp4` is the only path that starts the video view, and only when you submit it from the address bar. Other addresses stay on the page. |
| **A** | **Anti-fragile** | The program runs as the person who started it. It does not need a dedicated account. A missing page or a failed video returns control to that person instead of taking over the machine. |
| **O** | **Over-protect** | Critical window and player setup stays explicit. Do not remove a check, a log line, or a confirmation just to make the code shorter. |

Full principles: [CIAO Defensive Programming](https://github.com/cloudgen/ciao) · agent contract: [CIAO-Lite](https://github.com/cloudgen/ciao-lite).

This section describes **design posture**. It is **not** a claim of third-party certification (ISO, OWASP “compliant”, etc.).

## Scope notes

- Preferred language for reports: English.
- Out of scope: social engineering of third parties, physical attacks, spam, and vulnerabilities that exist only in the web page loaded inside the window.
- This program has no online installer and no companion checksum file. Build and run steps are in [`README.md`](README.md).
- Related product docs: [`README.md`](README.md), [`LICENSE.md`](LICENSE.md).
