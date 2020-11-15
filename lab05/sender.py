import argparse
import datetime
from email.message import EmailMessage
from getpass import getpass
import mimetypes
import os
import signal
import smtplib
import socket
from sys import stdin
import time


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='SMTP client')

    parser.add_argument('smtp', metavar='SMTP', type=str,
                        help='a SMTP server')
    parser.add_argument('login', metavar='login', type=str,
                        help='a login on SMTP server (from)')
    parser.add_argument('interval', metavar='interval', type=int, nargs='?',
                        help='an interval for sending emails (in seconds)')

    return parser.parse_args()


def send_email(server: smtplib.SMTP_SSL, msg: EmailMessage) -> None:
    try:
        server.send_message(msg)
        print('[%s] email was sent successfully' % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    except smtplib.SMTPRecipientsRefused:
        print('Can\'t sent email')
        exit(3)


def sigint_handler(sig, frame) -> None:
    print('\nSending emails stopped')
    exit(0)


def main() -> None:
    args = parse_args()

    server = None
    try:
        server = smtplib.SMTP_SSL(args.smtp)
    except socket.gaierror:
        print('SMTP server unavailable')
        exit(1)
    password = getpass()
    try:
        server.login(args.login, password)
    except smtplib.SMTPAuthenticationError:
        print('Unable to log in')
        exit(2)

    msg = EmailMessage()

    msg['From'] = args.login
    print(f'\nFrom: %s' % args.login)
    msg['To'] = str(input('To: '))
    msg['Subject'] = str(input('Subject: '))

    print('\nBody:')
    msg.set_content(stdin.read())
    print('')

    attachment = input('Attach: ').split()
    for filename in attachment:
        path = os.path.join(filename)
        if not os.path.isfile(path):
            print(f'\tCan\'t attached \'%s\'' % path)
            continue
        filetype, encoding = mimetypes.guess_type(path)
        if filetype is None or encoding is not None:
            filetype = 'application/octet-stream'
        maintype, subtype = filetype.split('/', 1)
        with open(path, 'rb') as fp:
            msg.add_attachment(fp.read(), maintype=maintype, subtype=subtype, filename=filename)
            print(f'\t\'%s\' successful attached' % filename)
    print('')

    if args.interval is None or args.interval == 0:
        send_email(server, msg)
    else:
        print(f'Sending emails at %d-second intervals. Press Ctrl+C to stop..' % args.interval)
        signal.signal(signal.SIGINT, sigint_handler)
        while True:
            send_email(server, msg)
            time.sleep(args.interval)


if __name__ == '__main__':
    main()
