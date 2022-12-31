import commonjs from '@rollup/plugin-commonjs';
import nativePlugin from 'rollup-plugin-natives';
import typescript from 'rollup-plugin-typescript2';
import { nodeResolve } from '@rollup/plugin-node-resolve';
import { builtinModules } from 'module';

export default {
  input: 'src/index.ts',
  output: {
    file: 'dist/index.js',
    format: 'cjs',
  },
  external: [...builtinModules],
  plugins: [
    commonjs(),
    nodeResolve(),
    typescript({
      tsconfig: 'tsconfig.build.json',
    }),
    nativePlugin({
      copyTo: 'dist/native',
      destDir: './native',
      dlopen: false,
      sourcemap: true
    })
  ],
};