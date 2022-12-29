module.exports = {
    testEnvironment: "node",
    roots: ["<rootDir>/src"],
    verbose: true,
    testMatch: ["**/?(*.)+(spec|test).+(ts|tsx)"],
    transform: {
      "^.+\\.(ts|tsx)$": "ts-jest",
    },
    coverageDirectory: "./test-reports",
    collectCoverageFrom: ["src/**/*.ts"],
    moduleNameMapper: {
      "^src/(.*)$": "<rootDir>/src/$1",
    },
  };
  