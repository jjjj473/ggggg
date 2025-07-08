import sys
from pathlib import Path

UTILITY_FUNCS = """
function uxcThrottle(fn, delay) {
  let last = 0;
  return function(...args) {
    const now = Date.now();
    if (now - last >= delay) {
      last = now;
      return fn.apply(this, args);
    }
  };
}

function uxcDebounce(fn, delay) {
  let timer;
  return function(...args) {
    clearTimeout(timer);
    timer = setTimeout(() => fn.apply(this, args), delay);
  };
}
function uxcUtil1(){}
function uxcUtil2(){}
function uxcUtil3(){}
function uxcUtil4(){}
function uxcUtil5(){}
function uxcUtil6(){}
function uxcUtil7(){}
function uxcUtil8(){}
function uxcUtil9(){}
function uxcUtil10(){}
function uxcUtil11(){}
function uxcUtil12(){}
function uxcUtil13(){}
function uxcUtil14(){}
function uxcUtil15(){}
function uxcUtil16(){}
function uxcUtil17(){}
function uxcUtil18(){}
function uxcUtil19(){}
function uxcUtil20(){}
function uxcUtil21(){}
function uxcUtil22(){}
function uxcUtil23(){}
function uxcUtil24(){}
function uxcUtil25(){}
function uxcUtil26(){}
function uxcUtil27(){}
function uxcUtil28(){}
function uxcUtil29(){}
function uxcUtil30(){}
function uxcUtil31(){}
function uxcUtil32(){}
function uxcUtil33(){}
function uxcUtil34(){}
function uxcUtil35(){}
function uxcUtil36(){}
function uxcUtil37(){}
function uxcUtil38(){}
function uxcUtil39(){}
function uxcUtil40(){}
function uxcUtil41(){}
function uxcUtil42(){}
function uxcUtil43(){}
function uxcUtil44(){}
function uxcUtil45(){}
function uxcUtil46(){}
function uxcUtil47(){}
function uxcUtil48(){}
function uxcUtil49(){}
function uxcUtil50(){}
function uxcUtil51(){}
function uxcUtil52(){}
function uxcUtil53(){}
function uxcUtil54(){}
function uxcUtil55(){}
function uxcUtil56(){}
function uxcUtil57(){}
function uxcUtil58(){}
function uxcUtil59(){}
function uxcUtil60(){}
function uxcUtil61(){}
function uxcUtil62(){}
function uxcUtil63(){}
function uxcUtil64(){}
function uxcUtil65(){}
function uxcUtil66(){}
function uxcUtil67(){}
function uxcUtil68(){}
function uxcUtil69(){}
function uxcUtil70(){}
"""

def parse_view(lines, start, scripts, counter):
    html = []
    stack = []
    void_tags = {"input", "img", "br", "hr", "meta", "link"}
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
            split_pos = content.rfind(': ')
            if split_pos == -1 and content.endswith(':'):
                split_pos = len(content) - 1
            if split_pos != -1:
                tag_part = content[:split_pos]
                text = content[split_pos + 1:].strip()
            else:
                tag_part, text = content, ''
        else:
            tag_part, text = content, ''

        parts = tag_part.split()
        tag = parts[0]
        attrs = {}
        bind_attrs = []
        cond = None
        style_bind = None
        class_bind = None
        text_bind = None
        html_bind = None
        model_bind = None
        show_cond = None
        each_expr = None
        fetch_url = None
        persist_key = None
        events = []
        watch_expr = None
        compute_expr = None
        prop_binds = []
        value_bind = None
        toggle_attrs = []
        hide_cond = None
        focus_flag = False
        blur_flag = False
        prepend_html = None
        append_html = None
        replace_html = None
        dispatch_evt = None
        oncreate_fn = None
        attr_once = []
        toggle_class = None
        append_text = None
        prepend_text = None
        scroll_flag = False
        dataset_binds = []
        ref_var = None
        before_html = None
        after_html = None
        
        for token in parts[1:]:
            if token.startswith(':') and '=' in token:
                name, val = token[1:].split('=', 1)
                if name == 'style':
                    style_bind = val
                elif name == 'class':
                    class_bind = val
                elif name == 'text':
                    text_bind = val
                elif name == 'html':
                    html_bind = val
                elif name == 'model':
                    model_bind = val
                elif name == 'value':
                    value_bind = val
                elif name == 'attrOnce':
                    attr, expr = val.split(':',1)
                    attr_once.append((attr, expr))
                elif name == 'toggleClass':
                    cls, cond_expr = val.split(':',1)
                    toggle_class = (cls, cond_expr)
                else:
                    bind_attrs.append((name, val))
            elif token.startswith('@') and '=' in token:
                k, v = token[1:].split('=', 1)
                prop_binds.append((k, v))
            elif token.startswith('if='):
                cond = token[3:]
            elif token.startswith('show='):
                show_cond = token[5:]
            elif token.startswith('hide='):
                hide_cond = token[5:]
            elif token.startswith('each='):
                each_expr = token[5:]
            elif token.startswith('fetch='):
                fetch_url = token[6:]
            elif token.startswith('persist='):
                persist_key = token[8:]
            elif token.startswith('watch='):
                watch_expr = token[6:]
            elif token.startswith('compute='):
                compute_expr = token[8:]
            elif token == 'focus':
                focus_flag = True
            elif token == 'blur':
                blur_flag = True
            elif token.startswith('prepend='):
                prepend_html = token[8:]
            elif token.startswith('append='):
                append_html = token[7:]
            elif token.startswith('replace='):
                replace_html = token[8:]
            elif token.startswith('dispatch='):
                dispatch_evt = token[9:]
            elif token.startswith('oncreate='):
                oncreate_fn = token[9:]
            elif token.startswith('appendText='):
                append_text = token[11:]
            elif token.startswith('prependText='):
                prepend_text = token[12:]
            elif token == 'scroll':
                scroll_flag = True
            elif token.startswith('data-') and '=' in token:
                k, v = token.split('=', 1)
                dataset_binds.append((k[5:], v))
            elif token.startswith('before='):
                before_html = token[7:]
            elif token.startswith('after='):
                after_html = token[6:]
            elif token.startswith('ref='):
                ref_var = token[4:]
            elif token.startswith('on') and '=' in token:
                event_part, handler = token.split('=', 1)
                event_name, *mods = event_part[2:].split('.')
                events.append((event_name, handler, mods))
            elif '=' in token:
                k, v = token.split('=', 1)
                attrs[k] = v
            else:
                attrs[token] = None

        if (bind_attrs or cond or style_bind or class_bind or text_bind or html_bind or
                model_bind or value_bind or show_cond or hide_cond or each_expr or fetch_url or
                persist_key or events or watch_expr or compute_expr or prop_binds or toggle_attrs or
                prepend_html or append_html or replace_html or dispatch_evt or oncreate_fn or
                append_text or prepend_text or scroll_flag or dataset_binds or before_html or after_html or
                attr_once or toggle_class or ref_var) and 'id' not in attrs:
            attrs['id'] = f'uxc{counter[0]}'
            counter[0] += 1

        open_tag = ' ' * indent + f'<{tag}'
        for k, v in attrs.items():
            if v is None:
                open_tag += f' {k}'
            else:
                open_tag += f' {k}={v}'
        if text:
            open_tag += f'>{text}</{tag}>'
            html.append(open_tag)
        elif tag in void_tags:
            open_tag += ' />'
            html.append(open_tag)
        else:
            open_tag += '>'
            html.append(open_tag)
            stack.append((indent, tag))

        elem_id = attrs.get('id')

        if bind_attrs:
            for attr, expr in bind_attrs:
                scripts.append(
                    f"document.getElementById('{elem_id}').setAttribute('{attr}', {expr});"
                )
        if style_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').style.cssText = {style_bind};"
            )
        if class_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').className = {class_bind};"
            )
        if text_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').textContent = {text_bind};"
            )
        if html_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').innerHTML = {html_bind};"
            )
        if model_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').value = {model_bind};"
            )
            scripts.append(
                f"document.getElementById('{elem_id}').addEventListener('input', e => {{ {model_bind} = e.target.value; }});"
            )
        if cond:
            scripts.append(
                f"if(!({cond})) document.getElementById('{elem_id}').remove();"
            )
        if show_cond:
            scripts.append(
                f"if(!({show_cond})) document.getElementById('{elem_id}').style.display='none';"
            )
        if fetch_url:
            scripts.append(
                f"fetch({fetch_url}).then(r=>r.text()).then(t=>{{document.getElementById('{elem_id}').innerHTML=t;}});"
            )
        if persist_key:
            scripts.append(
                f"document.getElementById('{elem_id}').value = localStorage.getItem('{persist_key}') || '';"
            )
            scripts.append(
                f"document.getElementById('{elem_id}').addEventListener('input', e => localStorage.setItem('{persist_key}', e.target.value));"
            )
        if watch_expr:
            var_name, cb = watch_expr.split(':',1)
            scripts.append(
                f"let __old_{var_name} = {var_name}; setInterval(()=>{{ if(__old_{var_name} !== {var_name}){{ __old_{var_name} = {var_name}; {cb}({var_name}); }} }},50);"
            )
        if compute_expr:
            var_name, expr = compute_expr.split(':',1)
            scripts.append(
                f"var {var_name} = {expr};"
            )
        if prop_binds:
            for prop, expr in prop_binds:
                scripts.append(
                    f"document.getElementById('{elem_id}').{prop} = {expr};"
                )
        if value_bind:
            scripts.append(
                f"document.getElementById('{elem_id}').value = {value_bind};"
            )
        if toggle_attrs:
            for attr, cond_expr in toggle_attrs:
                scripts.append(
                    f"if({cond_expr}) document.getElementById('{elem_id}').setAttribute('{attr}',''); else document.getElementById('{elem_id}').removeAttribute('{attr}');"
                )
        if hide_cond:
            scripts.append(
                f"if({hide_cond}) document.getElementById('{elem_id}').style.display='none';"
            )
        if focus_flag:
            scripts.append(
                f"document.getElementById('{elem_id}').focus();"
            )
        if blur_flag:
            scripts.append(
                f"document.getElementById('{elem_id}').blur();"
            )
        if prepend_html:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentHTML('afterbegin', {prepend_html});"
            )
        if append_html:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentHTML('beforeend', {append_html});"
            )
        if replace_html:
            scripts.append(
                f"document.getElementById('{elem_id}').innerHTML = {replace_html};"
            )
        if append_text:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentText('beforeend', {append_text});"
            )
        if prepend_text:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentText('afterbegin', {prepend_text});"
            )
        if dispatch_evt:
            scripts.append(
                f"document.getElementById('{elem_id}').dispatchEvent(new Event('{dispatch_evt}'));"
            )
        if oncreate_fn:
            scripts.append(
                f"{oncreate_fn}(document.getElementById('{elem_id}'));"
            )
        if scroll_flag:
            scripts.append(
                f"document.getElementById('{elem_id}').scrollIntoView();"
            )
        if dataset_binds:
            for key, expr in dataset_binds:
                scripts.append(
                    f"document.getElementById('{elem_id}').dataset['{key}'] = {expr};"
                )
        if before_html:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentHTML('beforebegin', {before_html});"
            )
        if after_html:
            scripts.append(
                f"document.getElementById('{elem_id}').insertAdjacentHTML('afterend', {after_html});"
            )
        if attr_once:
            for attr, expr in attr_once:
                scripts.append(
                    f"document.getElementById('{elem_id}').setAttribute('{attr}', {expr});"
                )
        if toggle_class:
            cls, cond_expr = toggle_class
            scripts.append(
                f"document.getElementById('{elem_id}').classList.toggle('{cls}', {cond_expr});"
            )
        if ref_var:
            scripts.append(
                f"window['{ref_var}'] = document.getElementById('{elem_id}');"
            )
        for evt, handler, mods in events:
            handler_code = ''
            options = []
            for m in mods:
                if m == 'prevent':
                    handler_code += 'event.preventDefault();'
                elif m == 'stop':
                    handler_code += 'event.stopPropagation();'
                elif m == 'once':
                    options.append('once: true')
                elif m == 'capture':
                    options.append('capture: true')
                elif m == 'passive':
                    options.append('passive: true')
                elif m.startswith('throttle'):
                    delay = m[len('throttle'):]
                    handler = f"uxcThrottle({handler}, {delay})"
                elif m.startswith('debounce'):
                    delay = m[len('debounce'):]
                    handler = f"uxcDebounce({handler}, {delay})"
                elif m.startswith('delay'):
                    delay = m[len('delay'):]
                    handler = f"(e=>setTimeout(()=>{handler}(e),{delay}))"
                elif m == 'log':
                    handler_code += 'console.log(event);'
            handler_code += f'{handler}(event);'
            opt = f", {{ {', '.join(options)} }}" if options else ''
            scripts.append(
                f"document.getElementById('{elem_id}').addEventListener('{evt}', function(event){{{handler_code}}}{opt});"
            )
        if each_expr:
            scripts.append(
                f"(() => {{const tpl=document.getElementById('{elem_id}');const p=tpl.parentNode;tpl.remove();for(const _ of {each_expr}){{const c=tpl.cloneNode(true);p.appendChild(c);}}}})();"
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
        print(UTILITY_FUNCS)
        if dynamic_scripts:
            print('\n'.join(dynamic_scripts))
        print('</script>')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python translator.py <file.uxc>')
        sys.exit(1)
    main(sys.argv[1])
