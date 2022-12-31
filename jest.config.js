module.exports = {
    testEnvironment: "node",
    roots: ["<rootDir>/src"],
    verbose: true,
    testMatch: ["**/?(*.)+(spec|test).+(ts|tsx)"],
    transform: {
      "^.+\\.(ts|tsx)$": "ts-jest",
    },
    coverageDirectory: "./test-reports",
    collectCoverageFrom: ["src/**/*.ts", "!src/test/scripts/**"],
    moduleNameMapper: {
      "^src/(.*)$": "<rootDir>/src/$1",
    },
    testPathIgnorePatterns: ["<rootDir>/node_modules/", "<rootDir>/dist/", "<rootDir>/test/scripts/"],
  };
  