import { nodeResolve } from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';

export default {
  input: 'src/js/index.js',
  output: {
    file: 'dist/bundle.js',
    format: 'iife',
    name: 'MiniReactNative',
    sourcemap: true,
    banner: '/* Mini React Native - JavaScript Bundle */',
    footer: '/* End of Bundle */'
  },
  plugins: [
    nodeResolve({
      browser: false,
      preferBuiltins: false
    }),
    commonjs({
      // 转换 CommonJS 模块
      transformMixedEsModules: true
    })
  ],
  // 确保全局变量在打包后仍然可用
  external: [],
  // 不处理外部依赖
  onwarn: (warning, warn) => {
    // 忽略一些常见的警告
    if (warning.code === 'THIS_IS_UNDEFINED') return;
    warn(warning);
  }
};