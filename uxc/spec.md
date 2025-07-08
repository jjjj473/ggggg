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

The prototype translator implements a couple of the planned systems:

* **Attribute binding** – specify `:attr=expr` on an element. The generated
  script sets the attribute value when the page loads.
* **Conditional rendering** – add `if=condition` to remove the element at load
  time when the condition evaluates to false.


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
