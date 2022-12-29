module.exports = {
    parser: "@typescript-eslint/parser",
    parserOptions: {
      ecmaVersion: 2018,
      project: "./tsconfig.json",
      sourceType: "module",
    },
    plugins: ["@typescript-eslint/eslint-plugin"],
    extends: [
      "plugin:@typescript-eslint/recommended",
      "plugin:prettier/recommended",
    ],
    root: true,
    env: {
      node: true,
      jest: true,
      es6: true,
    },
    ignorePatterns: [
      ".eslintrc.js",
      "dist/**/*",
      "node_modules/**/*",
      "rollup.config.js",
      "example/**/*",
      "index.ts",
      "**/*.md",
    ],
    rules: {
      "@typescript-eslint/interface-name-prefix": "on",
      "@typescript-eslint/explicit-function-return-type": "off",
      "@typescript-eslint/explicit-module-boundary-types": "off",
      "@typescript-eslint/no-explicit-any": "on",
      "@typescript-eslint/camelcase": "warn",
    },
  };
  