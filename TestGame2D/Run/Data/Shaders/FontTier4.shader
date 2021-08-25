<!-- 
   Common Attributes
   test: never, less, equal, less_equal, greater, greater_equal, not, always 
-->


<shader name="FontTier4">
   <!-- 
      Pass is a single draw call for this shader
      'cull': none, back, front
      'front': cw, ccw
      'fill': solid, wire
   -->

   <pass program="Data/Shaders/src/FontTier4.hlsl"
         front="ccw"
         cull="back"
         fill="solid">

	  <!-- blend: control blend modes
         enabled: to blend.  false is equivalent to "opaque"
         mode: blend mode. opaque, alpha, additive
      -->

      <blend enabled="false"
             mode="opaque" />

      <!-- depth state 
         enabled: depth check happens (false is similar to having test="always" and write being "false")
         test: comparison check 
         write: does this shader change depth
      -->

      <depth enabled="true"
             test="less_equal"
             write="true" />

   </pass>
</shader>





