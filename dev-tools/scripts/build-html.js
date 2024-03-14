/* eslint-disable camelcase, max-lines-per-function, jsdoc/require-jsdoc, jsdoc/require-param-description */
import Prism from 'prismjs'
import { minimatch } from 'minimatch'
import { minify } from 'html-minifier'
import { imageSize } from 'image-size'
import { JSDOM } from 'jsdom'
import { marked } from 'marked'
import { existsSync } from 'node:fs'
import { readdir, readFile } from 'node:fs/promises'
import { resolve, relative } from 'node:path'

const dom = new JSDOM('', {
  url: import.meta.url,
})
/** @type {Window} */
const window = dom.window
const document = window.document
const DOMParser = window.DOMParser

globalThis.window = dom.window
globalThis.document = document

if (document == null) {
  throw new Error('error parsing document')
}
// @ts-ignore
await import('prismjs/plugins/keep-markup/prism-keep-markup.js')
// @ts-ignore
await import('prismjs/components/prism-json.js')
await import('prismjs/components/prism-bash.js')

const projectPath = new URL('../../', import.meta.url)
const docsPath = new URL('docs', projectPath).pathname
const docsOutputPath = new URL('build-docs', projectPath).pathname

const fs = await import('fs')

const data = fs.readFileSync(`${docsPath}/${process.argv[2]}`, 'utf8')

const parsed = new DOMParser().parseFromString(data, 'text/html')
document.replaceChild(parsed.documentElement, document.documentElement)

const exampleCode = (strings, ...expr) => {
  let statement = strings[0]

  for (let i = 0; i < expr.length; i++) {
    statement += String(expr[i]).replace(/</g, '&lt')
    statement += strings[i + 1]
  }
  return statement
}

/**
 * @param {string} selector
 * @returns {Element[]} element array to use array methods
 */
const queryAll = (selector) => [...document.documentElement.querySelectorAll(selector)]

const readFileImport = (file) => existsSync(`${docsOutputPath}/${file}`) ? fs.readFileSync(`${docsOutputPath}/${file}`, 'utf8') : fs.readFileSync(`${docsPath}/${file}`, 'utf8')

const promises = []

queryAll('script.html-example').forEach(element => {
  const pre = document.createElement('pre')
  pre.innerHTML = exampleCode`<code class="language-markup keep-markup">${dedent(element.innerHTML)}</code>`
  element.replaceWith(pre)
})

queryAll('script.css-example').forEach(element => {
  const pre = document.createElement('pre')
  pre.innerHTML = exampleCode`<code class="language-css keep-markup">${dedent(element.innerHTML)}</code>`
  element.replaceWith(pre)
})

queryAll('script.json-example').forEach(element => {
  const pre = document.createElement('pre')
  pre.innerHTML = exampleCode`<code class="language-json keep-markup">${dedent(element.innerHTML)}</code>`
  element.replaceWith(pre)
})

queryAll('script.js-example').forEach(element => {
  const pre = document.createElement('pre')
  pre.innerHTML = exampleCode`<code class="language-js keep-markup">${dedent(element.innerHTML)}</code>`
  element.replaceWith(pre)
})

queryAll('svg[ss:include]').forEach(element => {
  const ssInclude = element.getAttribute('ss:include')
  const svgText = readFileImport(ssInclude)
  element.outerHTML = svgText
})

queryAll('[ss:markdown]:not([ss:include])').forEach(element => {
  const md = dedent(element.innerHTML)
    .replaceAll('\n&gt;', '\n>') // for blockquotes, innerHTML escapes ">" chars
  console.error(md)
  element.innerHTML = marked(md, { mangle: false, headerIds: false })
})

queryAll('[ss:markdown][ss:include]').forEach(element => {
  const ssInclude = element.getAttribute('ss:include')
  const md = readFileImport(ssInclude)
  element.innerHTML = marked(md, { mangle: false, headerIds: false })
})

queryAll('code').forEach(element => {
  Prism.highlightElement(element, false)
})

queryAll('img[ss:size]').forEach(element => {
  const imageSrc = element.getAttribute('src')
  const size = imageSize(`${docsOutputPath}/${imageSrc}`)
  element.removeAttribute('ss:size')
  element.setAttribute('width', `${size.width}`)
  element.setAttribute('height', `${size.height}`)
})

promises.push(...queryAll('img[ss:badge-attrs]').map(async (element) => {
  const imageSrc = element.getAttribute('src')
  const svgText = await readFile(`${docsOutputPath}/${imageSrc}`, 'utf8')
  const div = document.createElement('div')
  div.innerHTML = svgText
  element.removeAttribute('ss:badge-attrs')
  const svg = div.querySelector('svg')
  if (!svg) { throw Error(`${docsOutputPath}/${imageSrc} is not a valid svg`) }

  const alt = svg.getAttribute('aria-label')
  if (alt) { element.setAttribute('alt', alt) }

  const title = svg.querySelector('title')?.textContent
  if (title) { element.setAttribute('title', title) }
}))

queryAll('link[href][rel="stylesheet"][ss:inline]').forEach(element => {
  const href = element.getAttribute('href')
  const cssText = readFileImport(href)
  element.outerHTML = `<style>${cssText}</style>`
})

promises.push(...queryAll('link[href][ss:repeat-glob]').map(async (element) => {
  const href = element.getAttribute('href')
  if (!href) { return }
  for await (const filename of getFiles(docsOutputPath)) {
    const relativePath = relative(docsOutputPath, filename)
    if (!minimatch(relativePath, href)) { continue }
    const link = document.createElement('link')
    for (const { name, value } of element.attributes) {
      link.setAttribute(name, value)
    }
    link.removeAttribute('ss:repeat-glob')
    link.setAttribute('href', filename)
    element.insertAdjacentElement('afterend', link)
  }
  element.remove()
}))

const tocUtils = {
  getOrCreateId: (element) => {
    const id = element.getAttribute('id') || element.textContent.trim().toLowerCase().replaceAll(/\s+/g, '-')
    if (!element.hasAttribute('id')) {
      element.setAttribute('id', id)
    }
    return id
  },
  createMenuItem: (element) => {
    const a = document.createElement('a')
    const li = document.createElement('li')
    a.href = `#${element.id}`
    a.textContent = element.textContent
    li.append(a)
    return li
  },
  getParentOL: (element, path) => {
    while (path.length > 0) {
      const [title, possibleParent] = path.at(-1)
      if (title.tagName < element.tagName) {
        const possibleParentList = possibleParent.querySelector('ol')
        if (!possibleParentList) {
          const ol = document.createElement('ol')
          possibleParent.append(ol)
          return ol
        }
        return possibleParentList
      }
      path.pop()
    }
    return null
  },
}

await Promise.all(promises)

queryAll('[ss:toc]').forEach(element => {
  const ol = document.createElement('ol')
  /** @type {[HTMLElement, HTMLElement][]} */
  const path = []
  for (const element of queryAll(':is(h2, h3, h4, h5, h6):not(.no-toc), h1.yes-toc')) {
    tocUtils.getOrCreateId(element)
    const parent = tocUtils.getParentOL(element, path) || ol
    const li = tocUtils.createMenuItem(element)
    parent.append(li)
    path.push([element, li])
  }
  element.replaceWith(ol)
})

const minifiedHtml = minify('<!DOCTYPE html>' + document.documentElement?.outerHTML || '', {
  removeAttributeQuotes: true,
  useShortDoctype: true,
  collapseWhitespace: false,
})

fs.writeFileSync(`${docsOutputPath}/${process.argv[2]}`, minifiedHtml)

function dedent (templateStrings, ...values) {
  const matches = []
  const strings = typeof templateStrings === 'string' ? [templateStrings] : templateStrings.slice()
  strings[strings.length - 1] = strings[strings.length - 1].replace(/\r?\n([\t ]*)$/, '')
  for (const string of strings) {
    const match = string.match(/\n[\t ]+/g)
    match && matches.push(...match)
  }
  if (matches.length) {
    const size = Math.min(...matches.map(value => value.length - 1))
    const pattern = new RegExp(`\n[\t ]{${size}}`, 'g')
    for (let i = 0; i < strings.length; i++) {
      strings[i] = strings[i].replace(pattern, '\n')
    }
  }

  strings[0] = strings[0].replace(/^\r?\n/, '')
  let string = strings[0]
  for (let i = 0; i < values.length; i++) {
    string += values[i] + strings[i + 1]
  }
  return string
}

async function * getFiles (dir) {
  const dirents = await readdir(dir, { withFileTypes: true })

  for (const dirent of dirents) {
    const res = resolve(dir, dirent.name)
    if (dirent.isDirectory()) {
      yield * getFiles(res)
    } else {
      yield res
    }
  }
}
