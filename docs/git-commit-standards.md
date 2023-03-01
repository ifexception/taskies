# Taskies Git Commit Standards

Based of this article: https://www.conventionalcommits.org/en/v1.0.0

## Structure
The commit message should be structured as follows:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

## Elements

### fix:
a commit of type `fix` patches a bug in your codebase (correlates with `PATCH`)

### feat:
a commit of type `feat` introduces a new feature to your codebase (correlates with `MINOR`)

### BREAKING CHANGE
a commit that has a footer `BREAKING CHANGE`, introduces breaking API/flow changes (correlates with `MAJOR`)

_BREAKING CHANGE can only be part of a `fix` or `feat` type_

### build:
a commit of type `build` add/amends a change in the build process

### ci:
a commit of type `ci` adds/amends a change to your continuous integration pipeline

### docs:
a commit of type `docs` adds/amends the documentation of your codebase

### refactor:
a commit of type `refactor` amends/cleans up a feature, it can also include formatting, style changes

### test:
a commit of type `test` adds a test(s) to your codebase
