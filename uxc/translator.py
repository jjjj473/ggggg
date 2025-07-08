import sys
from pathlib import Path


def parse_view(lines, start, scripts, counter):
    html = []
    stack = []
    i = start
    while i < len(lines):
        line = lines[i]
        if not line.strip():
            i += 1
            continue
        if line.strip() == 'logic:' or line.startswith('component '):
            break
        indent = len(line) - len(line.lstrip())
        content = line.strip()
        if ':' in content:
            tag_part, text = content.rsplit(':', 1)
            text = text.strip()
        else:
            tag_part, text = content, ''

        parts = tag_part.split()
        tag = parts[0]
        attrs = {}
        bind_attrs = []
        cond = None

        for token in parts[1:]:
            if token.startswith(':') and '=' in token:
                # attribute binding :attr=expr
                a, val = token[1:].split('=', 1)
                bind_attrs.append((a, val))
            elif token.startswith('if='):
                cond = token[3:]
            elif '=' in token:
                k, v = token.split('=', 1)
                attrs[k] = v
            else:
                attrs[token] = None

        if (bind_attrs or cond) and 'id' not in attrs:
            attrs['id'] = f'uxc{counter[0]}'
            counter[0] += 1

        open_tag = ' ' * indent + f'<{tag}'
        for k, v in attrs.items():
            if v is None:
                open_tag += f' {k}'
            else:
                open_tag += f' {k}={v}'
        open_tag += '>'
        if text:
            open_tag += text
            html.append(open_tag + f'</{tag}>')
        else:
            html.append(open_tag)
            stack.append((indent, tag))

        if bind_attrs:
            for attr, expr in bind_attrs:
                scripts.append(
                    f"document.getElementById('{attrs['id']}').setAttribute('{attr}', {expr});"
                )
        if cond:
            scripts.append(
                f"if(!({cond})) document.getElementById('{attrs['id']}').remove();"
            )
        i += 1
        next_indent = len(lines[i]) - len(lines[i].lstrip()) if i < len(lines) else 0
        while stack and next_indent <= stack[-1][0]:
            ind, t = stack.pop()
            html.append(' ' * ind + f'</{t}>')
    while stack:
        ind, t = stack.pop()
        html.append(' ' * ind + f'</{t}>')
    return html, i


def main(path):
    data = Path(path).read_text().splitlines()
    i = 0
    html_lines = []
    js_lines = []
    dynamic_scripts = []
    counter = [0]
    while i < len(data):
        line = data[i]
        if line.startswith('component '):
            comp_name = line.split()[1]
            i += 1
            continue
        if line.strip() == 'view:':
            view_html, i = parse_view(data, i + 1, dynamic_scripts, counter)
            html_lines.extend(view_html)
            continue
        if line.strip() == 'logic:':
            i += 1
            while i < len(data):
                js_lines.append(data[i])
                i += 1
        i += 1

    print('\n'.join(html_lines))
    if js_lines or dynamic_scripts:
        print('<script>')
        if js_lines:
            print('\n'.join(js_lines))
        if dynamic_scripts:
            print('\n'.join(dynamic_scripts))
        print('</script>')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python translator.py <file.uxc>')
        sys.exit(1)
    main(sys.argv[1])
