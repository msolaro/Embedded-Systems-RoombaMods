/*
 * movement.c
 * created on 9/4/18
 *
 * @authors
 *  Max Solaro
 *  Eric Stablein
 */



#include "movement.h"
#include "open_interface.h"






void move_forward(oi_t *sensor, int centimeter)
{
    int remaining_distance;
      int sum=0;
      oi_setWheels(300,300);
      //roomba goes

      while(sum<(centimeter*10))
      {
          oi_update(sensor);
         sum+=sensor->distance;
         if(sensor->bumpLeft)//if its something goes around
         {
             remaining_distance= ((centimeter*10)-sum)/10;
             obstacle_left(sensor);
             move_forward(sensor,remaining_distance);
         }
         if(sensor->bumpRight)
         {
             remaining_distance= ((centimeter*10)-sum)/10; // jots down current distance and calculates remaining distance
             obstacle_right(sensor);
             move_forward(sensor,remaining_distance);
         }

      }

      oi_setWheels(0,0);
      //roomba stops

}
void move_backward(oi_t *sensor, int centimeter)
{

      int sum=0;
      oi_setWheels(-300,-300);
      //roomba goes

      while(sum<(centimeter*10))
      {
          oi_update(sensor);
         sum-=sensor->distance;
      }

      oi_setWheels(0,0);
      //roomba stops

}

void turn_counterclockwise(oi_t *sensor, int degrees)
{
    int sum=0;
    oi_setWheels(-300,300);

    while(sum<degrees)
    {
        oi_update(sensor);
        sum-=sensor->angle;

     /*   if(sensor->bumpLeft)
            obstacle_left(sensor);
        if(sensor->bumpRight)
          obstacle_right(sensor);
  */

    }

    oi_setWheels(0,0);
    //roomba stops


}
void turn_clockwise(oi_t *sensor, int degrees)
{
    int sum=0;
    oi_setWheels(300,-300);

    while(sum<degrees)
    {
        oi_update(sensor);
        sum+=sensor->angle;

    /*    if(sensor->bumpLeft)
            obstacle_left(sensor);
        if(sensor->bumpRight)
            obstacle_right(sensor);
*/

    }

    oi_setWheels(0,0);
    //roomba stops


}

void moveRectangle_counterclockwise(oi_t *sensor, int centimeter)
{
    int i=0;
    for(i=0;i<4;i++)
    {
        move_forward(sensor,centimeter);
        //goes straight
        turn_clockwise(sensor,90);
        //turns

    }
}


void obstacle_right(oi_t *sensor)
{
     move_backward(sensor,15);
     turn_clockwise(sensor,90);
     move_forward(sensor,25);
     turn_counterclockwise(sensor,90);
    // sensor->distance -=350;
}

void obstacle_left(oi_t *sensor)
{
    move_backward(sensor,15);
    turn_counterclockwise(sensor,90);
    move_forward(sensor,25);
    turn_clockwise(sensor,90);

   // sensor->distance -=350;

}


