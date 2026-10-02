# Security Policy

This library talks to vehicle control units over the CAN bus and can clear trouble codes. Security reports are therefore taken seriously.

## Supported Versions

Security fixes are applied to the latest code on the default branch and to the most recent release.

## Reporting a Vulnerability

If you find a security issue, **please do not open a public issue**.

Instead, email **muksin.muksin04@gmail.com** with:

- A description of the issue and its potential impact
- Steps to reproduce (board, configuration, sketch)
- A suggested fix, if you have one

You will receive a response as soon as possible, and credit in the release notes if you wish.

## Notes for Users

- The library sends whatever your sketch asks it to send. If your project exposes that over Wi-Fi, Bluetooth or another network, protecting that interface is the job of your project.
- `writeRawData()` puts any frame on the bus. Sending frames you do not understand to a moving vehicle is dangerous; test with the vehicle stationary.
