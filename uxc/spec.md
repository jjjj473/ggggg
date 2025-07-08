# UXC Language Specification

UXC is a minimal markup language designed to simplify building web UI components
and their associated logic. UXC files are plain text with two main sections:
`view` and `logic`.

## File Structure

```
component <Name>
view:
  <indent based markup>
logic:
  <JavaScript code>
```

* `component` declares the component's name.
* `view:` introduces the UI section.
* `logic:` introduces the JavaScript logic section.

## View Syntax

The view section uses indentation to express HTML hierarchy. Each line has the
following form:

```
<tag> [attr=value ...]: [text]
```

* `tag` – HTML tag name.
* `attr=value` – optional space separated attributes.
* `text` – optional inline text content.
* Indentation with spaces indicates nesting.
* Tags without text create opening tags that will be closed when the indentation
  level decreases.

Example:

```
div id=main:
  h1: Hello UXC
  button onclick=sayHello: Click me
```

The above converts to:

```
<div id="main">
  <h1>Hello UXC</h1>
  <button onclick="sayHello()">Click me</button>
</div>
```

## Logic Section

The logic block contains JavaScript code that will be placed inside a `<script>`
block of the generated page. Write plain JS here.

Example:

```
function sayHello() {
  alert('Hello from UXC!');
}
```

## Compilation

Use `translator.py` to convert a `.uxc` file to an HTML file:

```
python uxc/translator.py uxc/example.uxc > output.html
```

## Implemented Systems

The prototype translator currently supports forty of the proposed systems:

1. **Attribute binding** – specify `:attr=expr` on an element to set an
   attribute when the page loads.
2. **Two-way data binding** – `:model=var` on inputs keeps a variable and the
   input value in sync.
3. **Conditional rendering** – add `if=condition` to remove the element when the
   condition is false.
4. **Loop rendering** – `each=item in items` duplicates the element for each
   array item.
5. **Event `.prevent` modifier** – prevents default behaviour before calling the
   handler.
6. **Event `.stop` modifier** – stops propagation before calling the handler.
7. **Event `.once` modifier** – listener removed after first invocation.
8. **Event `.capture` modifier** – listener registered for the capture phase.
9. **Event `.passive` modifier** – listener marked as passive.
10. **Event `.throttleN` modifier** – wraps the handler with a throttle helper.
11. **Event `.debounceN` modifier** – wraps the handler with a debounce helper.
12. **Style binding** – `:style=expr` sets `style.cssText` dynamically.
13. **Class binding** – `:class=expr` sets the element class string.
14. **Text binding** – `:text=expr` assigns `textContent`.
15. **HTML binding** – `:html=expr` assigns `innerHTML`.
16. **Show/hide** – `show=condition` toggles `display:none`.
17. **Fetch helper** – `fetch=url` replaces the element contents with the HTTP
    response.
18. **Local storage helper** – `persist=key` binds an input's value to
    `localStorage`.
19. **Watchers** – `watch=var:callback` invokes a function when a variable
    changes.
20. **Computed properties** – `compute=var:expr` declares a variable computed at
    load time.
21. **Property binding** – `@prop=expr` assigns a DOM property.
22. **Value binding** – `:value=expr` sets an element's value.
23. **Toggle attribute** – `toggle=attr:cond` adds or removes an attribute.
24. **Hide when** – `hide=cond` hides the element when true.
25. **Focus** – `focus` focuses the element on creation.
26. **Blur** – `blur` blurs the element on creation.
27. **Prepend HTML** – `prepend=expr` inserts HTML at the beginning.
28. **Append HTML** – `append=expr` inserts HTML at the end.
29. **Replace HTML** – `replace=expr` sets `innerHTML`.
30. **Dispatch events** – `dispatch=name` dispatches a custom event.
31. **On create hook** – `oncreate=fn` calls a function after creation.
32. **Event `.delayN` modifier** – delays the handler by N milliseconds.
33. **Event `.log` modifier** – logs the event before the handler.
34. **Attribute once** – `:attrOnce=name:expr` sets an attribute once.
35. **Toggle class** – `:toggleClass=cls:cond` toggles a class.
36. **Append text** – `appendText=expr` adds text inside the element.
37. **Prepend text** – `prependText=expr` adds text before content.
38. **Scroll** – `scroll` scrolls the element into view.
39. **Data binding** – `data-key=expr` assigns a dataset value.
40. **Element refs** – `ref=name` stores the element in `window[name]`.


## Built-in Systems and Logic

UXC aims to provide a large collection of optional systems that can be enabled by future tooling. These systems are not yet implemented in the prototype but are described here for completeness.

1. Attribute binding
2. Two-way data binding
3. Conditional rendering
4. Loop rendering
5. Event modifiers
6. Style binding
7. Component composition
8. Named slots
9. Props validation
10. Local component state
11. Computed properties
12. Watchers
13. Dependency injection
14. Router integration
15. HTTP service helpers
16. Persistent data store
17. Form helpers
18. Animation directives
19. Custom directives
20. Template partials
21. Internationalization
22. Theme switching
23. Local storage helpers
24. Authentication flows
25. Authorization guards
26. WebSocket channel helpers
27. GraphQL helpers
28. Worker thread utilities
29. Build pipeline configuration
30. Testing utilities
31. Command line interface
32. Plugin system
33. Linting rules
34. TypeScript support
35. Server-side rendering
36. Client-side hydration
37. Virtual DOM
38. Portal rendering
39. Event bus
40. Accessibility helpers
41. Finite state machine
42. Snapshot testing
43. Code splitting
44. Lazy loading
45. Error boundaries
46. Logging utilities
47. Telemetry reporting
48. Theme presets
49. Production build optimizer
50. Development hot reload
