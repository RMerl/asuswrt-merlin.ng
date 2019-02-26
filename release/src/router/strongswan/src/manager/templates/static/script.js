
$(function(){
  $(".expand > h1").toggle(
    function(){$(this).parent(".expand").find(".expander").slideUp('fast');},
    function(){$(this).parent(".expand").find(".expander").slideDown('fast');}
  );
});
